#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

#include "focasmonitor.h"
#include "fwlib32.h"

// Global variables for signal handling
static volatile bool g_running = true;
static ConnectionPool g_pool;

void signal_handler(int sig) {
  (void)sig; // Suppress unused parameter warning
  printf("\nShutting down FOCAS Monitor...\n");
  g_running = false;
}

void show_usage(const char *program_name) {
  printf("FOCAS Monitor - Multi-Machine FANUC CNC Monitoring\n");
  printf("Usage: %s [OPTIONS]\n\n", program_name);
  printf("Options:\n");
  printf("  --machines=<file>           Load machine list from configuration "
         "file\n");
  printf("  --add=<name,ip,port>        Add a machine (can be used multiple "
         "times)\n");
  printf("  --info=<type>               Information to display:\n");
  printf("                              all      - All available information "
         "(default)\n");
  printf(
      "                              basic    - Machine ID and status only\n");
  printf("                              program  - Program information\n");
  printf("                              position - Tool position data\n");
  printf("                              speed    - Speed and feed rate data\n");
  printf("                              alarm    - Alarm status\n");
  printf("  --monitor                   Continuous monitoring mode\n");
  printf("  --interval=<seconds>        Monitoring interval (default: 30 "
         "seconds)\n");
  printf("  --output=<format>           Output format: console, json, csv\n");
  printf("  --verbose                   Enable verbose logging\n");
  printf("  --diagnose                  Run network diagnostics on connection failures\n");
  printf("  --status                    Show connection pool status\n");
  printf("  --timeout=<seconds>         Connection timeout (default: 10 "
         "seconds)\n");
  printf("  --help                      Show this help message\n");
  printf("  --version                   Show version information\n\n");
  printf("Examples:\n");
  printf("  %s --machines=factory.txt --info=basic\n", program_name);
  printf(
      "  %s --add=Mill1,192.168.1.100,8193 --add=Lathe1,192.168.1.101,8193\n",
      program_name);
  printf("  %s --machines=machines.txt --monitor --interval=60\n",
         program_name);
  printf("  %s --machines=machines.txt --info=alarm --output=json\n",
         program_name);
}

void show_version(void) {
#ifdef BUILD_TIMESTAMP
  printf("FOCAS Monitor v2.0.0\n");
  printf("Built: %s\n", BUILD_TIMESTAMP);
  printf("Commit: %s\n", BUILD_COMMIT);
  printf("Platform: Windows 32-bit\n");
  printf("FOCAS: Official FANUC library\n");
#else
  printf("FOCAS Monitor v2.0.0\n");
  printf("Platform: Windows 32-bit\n");
  printf("FOCAS: Official FANUC library\n");
#endif
}

int read_config(int argc, char *argv[], Config *conf) {
  // Initialize default configuration
  memset(conf, 0, sizeof(Config));
  strcpy(conf->info_type, "all");
  strcpy(conf->output_format, "console");
  conf->monitor_interval = DEFAULT_MONITOR_INTERVAL;
  conf->timeout = CONNECTION_TIMEOUT;
  conf->verbose = false;
  conf->diagnose = false;
  conf->monitor_mode = false;
  conf->show_status = false;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--machines=", 11) == 0) {
      strncpy(conf->config_file, argv[i] + 11, sizeof(conf->config_file) - 1);
    } else if (strncmp(argv[i], "--info=", 7) == 0) {
      strncpy(conf->info_type, argv[i] + 7, sizeof(conf->info_type) - 1);
    } else if (strncmp(argv[i], "--output=", 9) == 0) {
      strncpy(conf->output_format, argv[i] + 9,
              sizeof(conf->output_format) - 1);
    } else if (strncmp(argv[i], "--interval=", 11) == 0) {
      conf->monitor_interval = atoi(argv[i] + 11);
      if (conf->monitor_interval < 1)
        conf->monitor_interval = DEFAULT_MONITOR_INTERVAL;
    } else if (strncmp(argv[i], "--timeout=", 10) == 0) {
      conf->timeout = atoi(argv[i] + 10);
      if (conf->timeout < 1)
        conf->timeout = CONNECTION_TIMEOUT;
    } else if (strcmp(argv[i], "--monitor") == 0) {
      conf->monitor_mode = true;
    } else if (strcmp(argv[i], "--verbose") == 0) {
      conf->verbose = true;
    } else if (strcmp(argv[i], "--diagnose") == 0) {
      conf->diagnose = true;
    } else if (strcmp(argv[i], "--status") == 0) {
      conf->show_status = true;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      return 1; // Signal to show help
    } else if (strcmp(argv[i], "--version") == 0 ||
               strcmp(argv[i], "-v") == 0) {
      return 2; // Signal to show version
    }
  }

  return 0;
}

int load_machines_from_file(const char *filename, ConnectionPool *pool) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: Cannot open machine file '%s'\n", filename);
    return -1;
  }

  char line[256];
  int line_num = 0;
  int machines_added = 0;

  while (fgets(line, sizeof(line), file)) {
    line_num++;

    // Skip empty lines and comments
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    // Remove trailing newline
    char *newline = strchr(line, '\n');
    if (newline)
      *newline = '\0';
    char *carriage = strchr(line, '\r');
    if (carriage)
      *carriage = '\0';

    // Parse machine specification
    char name[50], ip[100];
    int port;

    if (parse_machine_spec(line, name, ip, &port) == 0) {
      fprintf(stderr, "Warning: Invalid machine specification on line %d: %s\n",
              line_num, line);
      continue;
    }

    // Add machine to pool
    FocasResult result = connection_pool_add_machine(pool, name, ip, port);
    if (result != FOCAS_OK) {
      fprintf(stderr, "Warning: Failed to add machine %s (%s:%d): %s\n", name,
              ip, port, focas_result_to_string(result));
      continue;
    }

    machines_added++;
  }

  fclose(file);

  if (machines_added == 0) {
    fprintf(stderr, "Error: No valid machines found in file '%s'\n", filename);
    return -1;
  }

  printf("Loaded %d machines from '%s'\n", machines_added, filename);
  return machines_added;
}

int parse_machine_spec(const char *spec, char *name, char *ip, int *port) {
  return (sscanf(spec, "%49[^,],%99[^,],%d", name, ip, port) == 3);
}

int process_add_arguments(int argc, char *argv[], ConnectionPool *pool) {
  int machines_added = 0;

  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--add=", 6) == 0) {
      char name[50], ip[100];
      int port;

      if (parse_machine_spec(argv[i] + 6, name, ip, &port) == 0) {
        fprintf(stderr, "Warning: Invalid machine specification: %s\n",
                argv[i] + 6);
        continue;
      }

      FocasResult result = connection_pool_add_machine(pool, name, ip, port);
      if (result != FOCAS_OK) {
        fprintf(stderr, "Warning: Failed to add machine %s (%s:%d): %s\n", name,
                ip, port, focas_result_to_string(result));
        continue;
      }

      printf("Added machine: %s (%s:%d)\n", name, ip, port);
      machines_added++;
    }
  }

  return machines_added;
}

int monitor_machines(ConnectionPool *pool, Config *conf) {
  MultiMachineInfo multi_info;
  OutputFormat format = parse_output_format(conf->output_format);

  while (g_running) {
    // Read all machine information
    FocasResult result = connection_pool_read_all_info(pool, &multi_info);

    if (result == FOCAS_OK || multi_info.successful_reads > 0) {
      // Clear screen for console output (Windows)
      if (format == OUTPUT_CONSOLE) {
        system("cls");
        printf("FOCAS Monitor - %s\n", ctime(&multi_info.collection_time));
        printf("Machines: %d successful, %d failed\n\n",
               multi_info.successful_reads, multi_info.failed_reads);
      }

      print_multi_machine_info(&multi_info, conf->info_type, format);
    } else {
      if (conf->verbose) {
        printf("Failed to read machine information: %s\n",
               focas_result_to_string(result));
      }
    }

    // Wait for next cycle
    for (int i = 0; i < conf->monitor_interval && g_running; i++) {
      sleep(1);
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  Config conf;
  MultiMachineInfo multi_info;
  FocasResult result;

  // Setup signal handlers
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Check for help and version flags first
  int config_result = read_config(argc, argv, &conf);
  if (config_result == 1) {
    show_usage(argv[0]);
    return EXIT_SUCCESS;
  } else if (config_result == 2) {
    show_version();
    return EXIT_SUCCESS;
  }

  if (conf.verbose) {
    printf("FOCAS Monitor starting...\n");
    printf("Configuration:\n");
    printf("  Info Type: %s\n", conf.info_type);
    printf("  Output Format: %s\n", conf.output_format);
    printf("  Monitor Mode: %s\n", conf.monitor_mode ? "enabled" : "disabled");
    if (conf.monitor_mode) {
      printf("  Monitor Interval: %d seconds\n", conf.monitor_interval);
    }
    printf("  Connection Timeout: %d seconds\n", conf.timeout);
    printf("\n");
  }

  // Initialize connection pool
  result = connection_pool_init(&g_pool);
  if (result != FOCAS_OK) {
    fprintf(stderr, "Error: Failed to initialize connection pool: %s\n",
            focas_result_to_string(result));
    return EXIT_FAILURE;
  }

  // Load machines from file if specified
  if (strlen(conf.config_file) > 0) {
    if (load_machines_from_file(conf.config_file, &g_pool) < 0) {
      return EXIT_FAILURE;
    }
  }

  // Process --add arguments
  int added_machines = process_add_arguments(argc, argv, &g_pool);
  if (added_machines > 0 && conf.verbose) {
    printf("Added %d machines from command line\n", added_machines);
  }

  // Check if we have any machines
  if (g_pool.machine_count == 0) {
    fprintf(stderr, "Error: No machines specified. Use --machines=<file> or "
                    "--add=<name,ip,port>\n\n");
    show_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (conf.verbose) {
    printf("Total machines configured: %d\n\n", g_pool.machine_count);
  }

  // Show connection pool status if requested
  if (conf.show_status) {
    connection_pool_print_status(&g_pool);
    printf("\n");
  }

  // Connect to all machines
  printf("Connecting to %d machines...\n", g_pool.machine_count);
  result = connection_pool_connect_all(&g_pool, conf.diagnose || conf.verbose);
  
  // Count successful and failed connections
  int connected = 0, failed = 0;
  for (int i = 0; i < g_pool.machine_count; i++) {
    if (g_pool.machines[i].state == CONN_CONNECTED) {
      connected++;
    } else {
      failed++;
    }
  }
  
  if (connected > 0 && failed == 0) {
    printf("[OK] All %d machines connected successfully\n", connected);
  } else if (connected > 0 && failed > 0) {
    printf("WARNING: Partial success: %d/%d machines connected (%d failed)\n", 
           connected, g_pool.machine_count, failed);
    printf("  Monitoring will continue with available machines\n");
    printf("  Failed connections will be retried automatically\n");
  } else {
    printf("[FAIL] All connection attempts failed (%d machines unreachable)\n", failed);
    printf("  Check network connectivity and machine configurations\n");
    printf("  Monitoring will continue and retry connections automatically\n");
  }

  // Monitor machines
  if (conf.monitor_mode) {
    printf("Starting continuous monitoring (interval: %d seconds)\n",
           conf.monitor_interval);
    printf("Press Ctrl+C to stop monitoring...\n\n");

    result = monitor_machines(&g_pool, &conf);
  } else {
    // Single read
    printf("Reading machine information...\n\n");

    result = connection_pool_read_all_info(&g_pool, &multi_info);
    if (result == FOCAS_OK) {
      OutputFormat format = parse_output_format(conf.output_format);
      print_multi_machine_info(&multi_info, conf.info_type, format);
    } else {
      fprintf(stderr, "Error reading machine information: %s\n",
              focas_result_to_string(result));
    }
  }

  // Cleanup
  connection_pool_disconnect_all(&g_pool);
  connection_pool_cleanup(&g_pool);

  if (conf.verbose) {
    printf("\nFOCAS Monitor finished.\n");
  }

  return (result == FOCAS_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}
