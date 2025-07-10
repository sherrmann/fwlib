#ifndef FW_UTIL_H
#define FW_UTIL_H

#include "./config.h"
#include "fwlib32.h"

int retrieve_id(Config *, char *);

// Map FOCAS error code to human-readable string
const char *focas_error_message(int code);

#endif
