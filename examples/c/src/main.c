#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h" 
#include "machine_info.h"
#include "fwlib32.h"

int main(int argc, char *argv[]) {
  MachineInfo machine_info;
  Config conf;

  if (read_config(argc, argv, &conf)) {
    fprintf(stderr,
            "usage: %s --config=<path_to_config> --port=<device port> "
            "--ip=<device ip>\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  printf("connecting to machine at %s:%d...\n", conf.ip, conf.port);

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
