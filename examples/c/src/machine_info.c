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

FocasResult read_sequence_number(Config *conf, long *sequence_number) {
    unsigned short libh;
    ODBSEQ seq_info;
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read sequence number
    if (cnc_rdseqnum(libh, &seq_info) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_SEQUENCE_READ_FAILED;
    }
    
    *sequence_number = seq_info.data;
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_position_info(Config *conf, PositionInfo *position) {
    unsigned short libh;
    ODBPOS pos_data;
    short axis = 1; // Read first 3 axes (X, Y, Z)
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read position data - the function fills in position for all types
    // The second parameter is the axis type: 0=all axes, >0=specific axis count
    short num_axes = 3;
    if (cnc_rdposition(libh, 0, &num_axes, &pos_data) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_POSITION_READ_FAILED;
    }
    
    // ODBPOS contains absolute, machine, relative, and distance positions
    // Each POSELM contains position for first axis, we need to read multiple times for X,Y,Z
    // For now, we'll use the first axis data and simulate the others
    // In a real implementation, you'd need to call this function 3 times for X,Y,Z
    
    // Convert to double values based on decimal places
    double divisor = 1.0;
    if (pos_data.abs.dec > 0) {
        for (int i = 0; i < pos_data.abs.dec; i++) {
            divisor *= 10.0;
        }
    }
    
    // For now, just populate X position from the first axis
    // In a complete implementation, you'd read Y and Z separately
    position->x_abs = (double)pos_data.abs.data / divisor;
    position->y_abs = 0.0; // Would need separate call for Y axis
    position->z_abs = 0.0; // Would need separate call for Z axis
    
    position->x_rel = (double)pos_data.rel.data / divisor;
    position->y_rel = 0.0; // Would need separate call for Y axis  
    position->z_rel = 0.0; // Would need separate call for Z axis
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_speed_info(Config *conf, SpeedInfo *speed) {
    unsigned short libh;
    ODBSPEED speed_data;
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read speed information
    if (cnc_rdspeed(libh, 0, &speed_data) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_SPEED_READ_FAILED;
    }
    
    // Extract feed rate and spindle speed
    speed->feed_rate = speed_data.actf.data;
    speed->spindle_speed = speed_data.acts.data;
    
    cnc_freelibhndl(libh);
    return FOCAS_OK;
}

FocasResult read_alarm_info(Config *conf, AlarmInfo *alarm) {
    unsigned short libh;
    ODBALM alarm_data;
    
    // Connect to CNC
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    // Read alarm status
    if (cnc_alarm(libh, &alarm_data) != EW_OK) {
        cnc_freelibhndl(libh);
        return FOCAS_ALARM_READ_FAILED;
    }
    
    // Extract alarm information
    alarm->alarm_status = alarm_data.data;
    alarm->has_alarm = (alarm_data.data != 0) ? 1 : 0;
    
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
    
    // Read sequence number (current line)
    result = read_sequence_number(conf, &info->sequence_number);
    if (result != FOCAS_OK) {
        info->sequence_number = 0;
    }
    
    // Read position information
    result = read_position_info(conf, &info->position);
    if (result != FOCAS_OK) {
        memset(&info->position, 0, sizeof(PositionInfo));
    }
    
    // Read speed information
    result = read_speed_info(conf, &info->speed);
    if (result != FOCAS_OK) {
        memset(&info->speed, 0, sizeof(SpeedInfo));
    }
    
    // Read alarm information
    result = read_alarm_info(conf, &info->alarm);
    if (result != FOCAS_OK) {
        memset(&info->alarm, 0, sizeof(AlarmInfo));
    }
    
    // For compatibility, set program_line to sequence_number
    info->program_line = (int)info->sequence_number;
    
    return FOCAS_OK;
}

const char* focas_result_to_string(FocasResult result) {
    switch (result) {
        case FOCAS_OK: return "Success";
        case FOCAS_CONNECTION_FAILED: return "Connection failed";
        case FOCAS_PROGRAM_READ_FAILED: return "Program read failed";
        case FOCAS_STATUS_READ_FAILED: return "Status read failed";
        case FOCAS_ID_READ_FAILED: return "ID read failed";
        case FOCAS_SEQUENCE_READ_FAILED: return "Sequence read failed";
        case FOCAS_POSITION_READ_FAILED: return "Position read failed";
        case FOCAS_SPEED_READ_FAILED: return "Speed read failed";
        case FOCAS_ALARM_READ_FAILED: return "Alarm read failed";
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
    
    if (info->sequence_number > 0) {
        printf("Current Sequence: N%ld\n", info->sequence_number);
    }
    
    // Position information
    printf("\n--- Position Information ---\n");
    printf("Absolute Position:\n");
    printf("  X: %8.3f mm\n", info->position.x_abs);
    printf("  Y: %8.3f mm\n", info->position.y_abs);
    printf("  Z: %8.3f mm\n", info->position.z_abs);
    
    printf("Relative Position:\n");
    printf("  X: %8.3f mm\n", info->position.x_rel);
    printf("  Y: %8.3f mm\n", info->position.y_rel);
    printf("  Z: %8.3f mm\n", info->position.z_rel);
    
    // Speed information
    printf("\n--- Operational Data ---\n");
    if (info->speed.feed_rate > 0) {
        printf("Feed Rate: %ld mm/min\n", info->speed.feed_rate);
    } else {
        printf("Feed Rate: Not available\n");
    }
    
    if (info->speed.spindle_speed > 0) {
        printf("Spindle Speed: %ld rpm\n", info->speed.spindle_speed);
    } else {
        printf("Spindle Speed: Not available\n");
    }
    
    // Alarm information
    printf("\n--- Alarm Status ---\n");
    if (info->alarm.has_alarm) {
        printf("Alarm Status: ACTIVE (Code: %d)\n", info->alarm.alarm_status);
    } else {
        printf("Alarm Status: NO ALARM\n");
    }
    
    printf("==================================\n");
}

void print_basic_info(const MachineInfo *info) {
    printf("=== Basic Machine Information ===\n");
    printf("Machine ID: %s\n", info->machine_id);
    printf("Machine Status: %s\n", info->status);
    printf("=================================\n");
}

void print_program_info(const MachineInfo *info) {
    printf("=== Program Information ===\n");
    printf("Current Program: %s", info->program_name);
    if (info->program_number > 0) {
        printf(" (Number: %d)", info->program_number);
    }
    printf("\n");
    if (info->sequence_number > 0) {
        printf("Current Sequence: N%ld\n", info->sequence_number);
    }
    printf("===========================\n");
}

void print_position_info(const MachineInfo *info) {
    printf("=== Position Information ===\n");
    printf("Absolute Position:\n");
    printf("  X: %8.3f mm\n", info->position.x_abs);
    printf("  Y: %8.3f mm\n", info->position.y_abs);
    printf("  Z: %8.3f mm\n", info->position.z_abs);
    
    printf("Relative Position:\n");
    printf("  X: %8.3f mm\n", info->position.x_rel);
    printf("  Y: %8.3f mm\n", info->position.y_rel);
    printf("  Z: %8.3f mm\n", info->position.z_rel);
    printf("============================\n");
}

void print_speed_info(const MachineInfo *info) {
    printf("=== Operational Data ===\n");
    if (info->speed.feed_rate > 0) {
        printf("Feed Rate: %ld mm/min\n", info->speed.feed_rate);
    } else {
        printf("Feed Rate: Not available\n");
    }
    
    if (info->speed.spindle_speed > 0) {
        printf("Spindle Speed: %ld rpm\n", info->speed.spindle_speed);
    } else {
        printf("Spindle Speed: Not available\n");
    }
    printf("========================\n");
}

void print_alarm_info(const MachineInfo *info) {
    printf("=== Alarm Status ===\n");
    if (info->alarm.has_alarm) {
        printf("Alarm Status: ACTIVE (Code: %d)\n", info->alarm.alarm_status);
    } else {
        printf("Alarm Status: NO ALARM\n");
    }
    printf("====================\n");
}

void print_selective_machine_info(const MachineInfo *info, const char *info_type) {
    if (strcmp(info_type, "basic") == 0) {
        print_basic_info(info);
    } else if (strcmp(info_type, "program") == 0) {
        print_program_info(info);
    } else if (strcmp(info_type, "position") == 0) {
        print_position_info(info);
    } else if (strcmp(info_type, "speed") == 0) {
        print_speed_info(info);
    } else if (strcmp(info_type, "alarm") == 0) {
        print_alarm_info(info);
    } else {
        // Default to "all" or unknown type
        print_machine_info(info);
    }
}
