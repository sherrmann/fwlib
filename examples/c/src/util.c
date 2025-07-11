#include "./util.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "./config.h"

// Map FOCAS error code to human-readable string
const char *focas_error_message(int code) {
  switch (code) {
  case 0:
    return "No error (EW_OK)";
  case -1:
    return "Busy error (EW_BUSY)";
  case -2:
    return "Reset or stop occurred (EW_RESET)";
  case -3:
    return "emm386 or mmcsys install error (EW_MMCSYS)";
  case -4:
    return "Shared RAM parity error (EW_PARITY)";
  case -5:
    return "System error (EW_SYSTEM)";
  case -6:
    return "Abnormal error (EW_UNEXP)";
  case -7:
    return "CNC/PMC version mismatch (EW_VERSION)";
  case -8:
    return "Windows library handle error (EW_HANDLE)";
  case -9:
    return "HSSB communication error (EW_HSSB)";
  case -10:
    return "System error (EW_SYSTEM2)";
  case -11:
    return "Bus error (EW_BUS)";
  case -12:
    return "High temperature alarm (EW_ITHIGHT)";
  case -13:
    return "Low temperature alarm (EW_ITLOW)";
  case -14:
    return "API library init file error (EW_INIERR)";
  case -15:
    return "DLL not exist error (EW_NODLL)";
  case -16:
    return "Windows socket error (EW_SOCKET) - Network/connection problem";
  case -17:
    return "Protocol error (EW_PROTOCOL)";
  default:
    return "Unknown FOCAS error code";
  }
}

int retrieve_id(Config *conf, char *cnc_id) {
  int allocated = 0;
  int ret = 0;
  unsigned short libh;
  uint32_t cnc_ids[4];

#ifndef _WIN32
  if (cnc_startupprocess(0, "focas.log") != EW_OK) {
    fprintf(stderr, "Failed to create required log file!\n");
    return 1;
  }
#endif

  printf("connecting to machine at %s:%d...\n", conf->ip, conf->port);
  if ((ret = cnc_allclibhndl3(conf->ip, conf->port, 10, &libh)) != EW_OK) {
    fprintf(stderr, "Failed to connect to cnc! (%d) - %s\n", ret,
            focas_error_message(ret));
    ret = 1;
    goto cleanup;
  }
  allocated = 1;

  if (cnc_rdcncid(libh, (unsigned long *)cnc_ids) != EW_OK) {
    fprintf(stderr, "Failed to read cnc id!\n");
    ret = 1;
    goto cleanup;
  }

  snprintf(cnc_id, 40, "%08x-%08x-%08x-%08x", cnc_ids[0], cnc_ids[1],
           cnc_ids[2], cnc_ids[3]);

cleanup:
  if (allocated && cnc_freelibhndl(libh) != EW_OK)
    fprintf(stderr, "Failed to free library handle!\n");
#ifndef _WIN32
  cnc_exitprocess();
#endif

  return ret;
}
