#ifndef MACHINE_INFO_H
#define MACHINE_INFO_H

#include "config.h"

// Machine information structure
typedef struct {
    char machine_id[36];
    char program_name[16];
    char status[16];
    int program_line;
    int program_number;
} MachineInfo;

// FOCAS result codes
typedef enum {
    FOCAS_OK = 0,
    FOCAS_CONNECTION_FAILED = -1,
    FOCAS_PROGRAM_READ_FAILED = -2,
    FOCAS_STATUS_READ_FAILED = -3,
    FOCAS_ID_READ_FAILED = -4
} FocasResult;

// Function declarations
FocasResult read_current_program(Config *conf, char *program_name, int *program_number);
FocasResult read_machine_status(Config *conf, char *status_info);
FocasResult read_machine_id(Config *conf, char *machine_id);
FocasResult read_complete_machine_info(Config *conf, MachineInfo *info);

// Helper functions
const char* focas_result_to_string(FocasResult result);
void print_machine_info(const MachineInfo *info);

#endif // MACHINE_INFO_H
