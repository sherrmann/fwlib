#ifndef MACHINE_INFO_H
#define MACHINE_INFO_H

#include "config.h"

// Position information structure
typedef struct {
    double x_abs;       // Absolute X position
    double y_abs;       // Absolute Y position  
    double z_abs;       // Absolute Z position
    double x_rel;       // Relative X position
    double y_rel;       // Relative Y position
    double z_rel;       // Relative Z position
} PositionInfo;

// Speed information structure
typedef struct {
    long feed_rate;     // Current feed rate
    long spindle_speed; // Current spindle speed
} SpeedInfo;

// Alarm information structure
typedef struct {
    int alarm_status;   // Overall alarm status (0=no alarm, >0=alarm active)
    int has_alarm;      // Boolean: 1 if any alarm is active, 0 if no alarms
} AlarmInfo;

// Machine information structure
typedef struct {
    char machine_id[36];
    char program_name[16];
    char status[16];
    int program_line;
    int program_number;
    long sequence_number;    // Current sequence number
    PositionInfo position;   // Tool position data
    SpeedInfo speed;         // Speed information
    AlarmInfo alarm;         // Alarm status
} MachineInfo;

// FOCAS result codes
typedef enum {
    FOCAS_OK = 0,
    FOCAS_CONNECTION_FAILED = -1,
    FOCAS_PROGRAM_READ_FAILED = -2,
    FOCAS_STATUS_READ_FAILED = -3,
    FOCAS_ID_READ_FAILED = -4,
    FOCAS_SEQUENCE_READ_FAILED = -5,
    FOCAS_POSITION_READ_FAILED = -6,
    FOCAS_SPEED_READ_FAILED = -7,
    FOCAS_ALARM_READ_FAILED = -8
} FocasResult;

// Function declarations
FocasResult read_current_program(Config *conf, char *program_name, int *program_number);
FocasResult read_machine_status(Config *conf, char *status_info);
FocasResult read_machine_id(Config *conf, char *machine_id);
FocasResult read_sequence_number(Config *conf, long *sequence_number);
FocasResult read_position_info(Config *conf, PositionInfo *position);
FocasResult read_speed_info(Config *conf, SpeedInfo *speed);
FocasResult read_alarm_info(Config *conf, AlarmInfo *alarm);
FocasResult read_complete_machine_info(Config *conf, MachineInfo *info);

// Helper functions
const char* focas_result_to_string(FocasResult result);
void print_machine_info(const MachineInfo *info);
void print_selective_machine_info(const MachineInfo *info, const char *info_type);

#endif // MACHINE_INFO_H
