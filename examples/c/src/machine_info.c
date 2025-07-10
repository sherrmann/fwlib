#include "machine_info.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Include the official FANUC header - should work with MinGW cross-compilation
#include "fwlib32.h"

FocasResult read_current_program(Config *conf, char *program_name, int *program_number) {
    unsigned short libh;
    ODBPRO prgnum;
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read current program number
    if (cnc_rdprgnum(libh, &prgnum) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_PROGRAM_READ_FAILED;
    }
    
    // Format program name (e.g., "O1234")
    snprintf(program_name, 16, "O%04d", prgnum.data);
    *program_number = (int)prgnum.data;
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_machine_status(Config *conf, char *status_info) {
    unsigned short libh;
    ODBST status;
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read machine status
    if (cnc_statinfo(libh, &status) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_STATUS_READ_FAILED;
    }
    
    // Interpret status
    switch(status.run) {
        case 0: strcpy(status_info, "STOPPED"); break;
        case 1: strcpy(status_info, "RUNNING"); break;
        case 2: strcpy(status_info, "PAUSED"); break;
        case 3: strcpy(status_info, "ALARM"); break;
        default: 
            snprintf(status_info, 16, "UNKNOWN(%d)", status.run);
            break;
    }
    
    // Add motion status if available
    if (status.motion == 1) {
        strcat(status_info, " (MOVING)");
    }
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_machine_id(Config *conf, char *machine_id) {
    unsigned short libh;
    unsigned long cncid[4];
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read CNC ID
    if (cnc_rdcncid(libh, cncid) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_ID_READ_FAILED;
    }
    
    // Format CNC ID
    snprintf(machine_id, 36, "%08lx-%08lx-%08lx-%08lx", 
             cncid[0], cncid[1], cncid[2], cncid[3]);
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_complete_machine_info(Config *conf, MachineInfo *info) {
    FocasResult result;
    
    // Initialize structure
    memset(info, 0, sizeof(MachineInfo));
    
    // Read machine ID
    result = read_machine_id(conf, info->machine_id);
    if (result != FOCAS_OK) {
        strcpy(info->machine_id, "UNKNOWN");
    }
    
    // Read current program
    result = read_current_program(conf, info->program_name, &info->program_number);
    if (result != FOCAS_OK) {
        strcpy(info->program_name, "UNKNOWN");
        info->program_number = 0;
    }
    
    // Read machine status
    result = read_machine_status(conf, info->status);
    if (result != FOCAS_OK) {
        strcpy(info->status, "UNKNOWN");
    }
    
    // For now, program line is not implemented
    info->program_line = 0;
    
    return FOCAS_OK;
}

const char* focas_result_to_string(FocasResult result) {
    switch (result) {
        case FOCAS_OK: return "Success";
        case FOCAS_CONNECTION_FAILED: return "Connection failed";
        case FOCAS_PROGRAM_READ_FAILED: return "Program read failed";
        case FOCAS_STATUS_READ_FAILED: return "Status read failed";
        case FOCAS_ID_READ_FAILED: return "ID read failed";
        default: return "Unknown error";
    }
}

void print_machine_info(const MachineInfo *info) {
    printf("=== FANUC Machine Information ===\n");
    printf("Machine ID: %s\n", info->machine_id);
    printf("Current Program: %s", info->program_name);
    if (info->program_number > 0) {
        printf(" (Number: %d)", info->program_number);
    }
    printf("\n");
    printf("Machine Status: %s\n", info->status);
    if (info->program_line > 0) {
        printf("Current Line: %d\n", info->program_line);
    }
    printf("==================================\n");
}
