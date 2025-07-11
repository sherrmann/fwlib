#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "connection_pool.h"

void show_multi_usage(const char *program_name) {
  printf("FANUC FOCAS Multi-Machine Demo\n");
  printf("Usage: %s [OPTIONS]\n\n", program_name);
  printf("Options:\n");
  printf("  --machines=<config_file>     Load machine list from file\n");
  printf("  --add=<name,ip,port>         Add a single machine\n");
  printf("  --info=<type>                Information to display:\n");
  printf(
      "                               all      - All available information\n");
  printf(
      "                               basic    - Machine ID and status only\n");
  printf("                               program  - Program information\n");
  printf("                               alarm    - Alarm status\n");
  printf("  --status                     Show connection pool status\n");
  printf("  --monitor                    Continuous monitoring mode\n");
  printf("  --help                       Show this help message\n\n");
  printf("Examples:\n");
  printf(
      "  %s --add=Mill1,192.168.1.100,8193 --add=Lathe1,192.168.1.101,8193\n",
      program_name);
  printf("  %s --machines=machine_list.txt --info=basic\n", program_name);
  printf("  %s --machines=machine_list.txt --monitor\n", program_name);
}

int parse_machine_spec(const char *spec, char *name, char *ip, int *port) {
  return (sscanf(spec, "%49[^,],%99[^,],%d", name, ip, port) == 3);
}

int main(int argc, char *argv[]) {
  ConnectionPool pool;
  MultiMachineInfo multi_info;
  char machines_file[256] = "";
  char info_type[20] = "all";
  int show_status = 0;
  int monitor_mode = 0;
  int machines_added = 0;

  // Check for help flag first
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      show_multi_usage(argv[0]);
      return EXIT_SUCCESS;
    }
  }

  // Initialize connection pool
  if (init_connection_pool(&pool) != FOCAS_OK) {
    printf("❌ Failed to initialize connection pool\n");
    return EXIT_FAILURE;
  }

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--machines=", 11) == 0) {
      strncpy(machines_file, argv[i] + 11, sizeof(machines_file) - 1);
    } else if (strncmp(argv[i], "--add=", 6) == 0) {
      char name[50], ip[100];
      int port;
      if (parse_machine_spec(argv[i] + 6, name, ip, &port)) {
        if (add_machine(&pool, ip, port, name) == FOCAS_OK) {
          machines_added++;
        }
      } else {
        printf("❌ Invalid machine spec: %s\n", argv[i] + 6);
        printf("   Format should be: name,ip,port\n");
      }
    } else if (strncmp(argv[i], "--info=", 7) == 0) {
      strncpy(info_type, argv[i] + 7, sizeof(info_type) - 1);
    } else if (strcmp(argv[i], "--status") == 0) {
      show_status = 1;
    } else if (strcmp(argv[i], "--monitor") == 0) {
      monitor_mode = 1;
    }
  }

  // Load machines from file if specified
  if (strlen(machines_file) > 0) {
    if (load_machine_list_from_config(machines_file, &pool) != FOCAS_OK) {
      printf("⚠️ Warning: Could not load all machines from config file\n");
    }
  }

  // Check if any machines were configured
  if (pool.machine_count == 0) {
    printf("❌ No machines configured. Use --add or --machines to specify "
           "machines.\n\n");
    show_multi_usage(argv[0]);
    return EXIT_FAILURE;
  }

  printf("🏭 Multi-machine FANUC monitoring initialized with %d machines\n\n",
         pool.machine_count);

  // Show connection pool status if requested
  if (show_status) {
    print_connection_pool_status(&pool);
  }

  if (monitor_mode) {
    printf("📊 Starting continuous monitoring mode (Ctrl+C to exit)...\n");
    printf("Press Enter to refresh, 'q' to quit\n\n");

    while (1) {
      // Clear screen (simple approach)
      printf("\033[2J\033[H");

      printf("🔄 FANUC Multi-Machine Monitor - %s",
             ctime(&(time_t){time(NULL)}));

      // Read all machines
      if (read_all_machines_info(&pool, &multi_info) == FOCAS_OK) {
        print_multi_machine_info(&multi_info, info_type);
      } else {
        printf("❌ Failed to read machine information\n");
      }

      // Simple input handling
      printf("\nPress Enter to refresh, 'q' + Enter to quit: ");
      char input[10];
      if (fgets(input, sizeof(input), stdin)) {
        if (input[0] == 'q' || input[0] == 'Q') {
          break;
        }
      }
    }
  } else {
    // Single read mode
    if (read_all_machines_info(&pool, &multi_info) == FOCAS_OK) {
      print_multi_machine_info(&multi_info, info_type);
    } else {
      printf("❌ Failed to read machine information\n");
      print_connection_pool_status(&pool);
    }
  }

  // Cleanup
  disconnect_all_machines(&pool);

  printf("\n✅ Multi-machine demo completed\n");
  return EXIT_SUCCESS;
}
