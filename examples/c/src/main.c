#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h" 
#include "machine_info.h"
#include "fwlib32.h"

void show_usage(const char *program_name) {
  printf("FANUC FOCAS Library Example\n");
  printf("Usage: %s [OPTIONS]\n\n", program_name);
  printf("Options:\n");
  printf("  --ip=<ip_address>        CNC machine IP address (required)\n");
  printf("  --port=<port_number>     CNC machine port (default: 8193)\n");
  printf("  --config=<config_file>   Read settings from config file\n");
  printf("  --help                   Show this help message\n");
  printf("  --version                Show version and build information\n\n");
  printf("Environment Variables:\n");
  printf("  DEVICE_IP                CNC machine IP address\n");
  printf("  DEVICE_PORT              CNC machine port\n\n");
  printf("Example:\n");
  printf("  %s --ip=192.168.1.100 --port=8193\n", program_name);
}

void show_version() {
#ifdef BUILD_TIMESTAMP
  printf("FANUC FOCAS Library Example\n");
  printf("Version: %s\n", BUILD_VERSION);
  printf("Built: %s\n", BUILD_TIMESTAMP);
  printf("Commit: %s\n", BUILD_COMMIT);
  printf("Compiler: MinGW-w64 GCC (cross-compiled)\n");
  printf("FOCAS: Official FANUC library\n");
#else
  printf("FANUC FOCAS Library Example\n");
  printf("Version: Development build\n");
  printf("Compiler: MinGW-w64 GCC (cross-compiled)\n");
  printf("FOCAS: Official FANUC library\n");
#endif
}

int main(int argc, char *argv[]) {
  MachineInfo machine_info;
  Config conf;

  // Check for help and version flags first
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      show_usage(argv[0]);
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
      show_version();
      return EXIT_SUCCESS;
    }
  }

  if (read_config(argc, argv, &conf)) {
    fprintf(stderr, "Error: Failed to read configuration.\n\n");
    show_usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Check if IP address was provided
  if (strlen(conf.ip) == 0) {
    printf("No FANUC machine IP address specified. Please provide one.\n\n");
    show_usage(argv[0]);
    printf("\nNote: Use --help for detailed usage information.\n");
    return EXIT_FAILURE;
  }

  // Show what configuration is being used
  printf("Connecting to FANUC machine at %s:%d...\n", conf.ip, conf.port);

  // Read complete machine information
  FocasResult result = read_complete_machine_info(&conf, &machine_info);
  
  if (result != FOCAS_OK) {
    printf("Warning: Some information may be incomplete (%s)\n", 
           focas_result_to_string(result));
  }

  // Display machine information
  print_machine_info(&machine_info);

  return EXIT_SUCCESS;
}
