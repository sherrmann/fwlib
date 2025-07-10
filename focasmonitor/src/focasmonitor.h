#ifndef FOCAS_MONITOR_H
#define FOCAS_MONITOR_H

#include <time.h>
#include <stdbool.h>

// Maximum number of machines per connection pool
#define MAX_MACHINES 50

// Connection timeout in seconds  
#define CONNECTION_TIMEOUT 10

// Default monitoring interval
#define DEFAULT_MONITOR_INTERVAL 30

// Configuration and machine data structures
typedef struct {
    char ip[100];
    int port;
    char config_file[256];
    char info_type[16];
    char output_format[16];
    bool verbose;
    bool monitor_mode;
    bool show_status;
    int monitor_interval;
    int timeout;
} Config;

// Position information
typedef struct {
    double x_abs;
    double y_abs;
    double z_abs;
    double x_rel;
    double y_rel;
    double z_rel;
} PositionInfo;

// Speed information
typedef struct {
    int feed_rate;
    int spindle_speed;
} SpeedInfo;

// Alarm information
typedef struct {
    int alarm_status;
    int has_alarm;
} AlarmInfo;

// Complete machine information
typedef struct {
    char machine_id[36];     // Machine identifier
    char program_name[16];   // O-number format  
    char status[16];         // RUNNING/STOPPED/PAUSED/ALARM
    int program_number;      // Numeric program ID
    long sequence_number;    // Current N-line
    int program_line;        // Compatibility field
    PositionInfo position;   // Tool position data
    SpeedInfo speed;         // Speed information
    AlarmInfo alarm;         // Alarm status
    time_t last_updated;     // When this info was collected
} MachineInfo;

// Connection states
typedef enum {
    CONN_DISCONNECTED = 0,
    CONN_CONNECTING = 1,
    CONN_CONNECTED = 2,
    CONN_ERROR = 3,
    CONN_BUSY = 4
} ConnectionState;

// Individual machine connection handle
typedef struct {
    char ip[100];
    int port;
    char friendly_name[50];
    unsigned short handle;
    ConnectionState state;
    time_t connect_time;
    time_t last_activity;
    int retry_count;
    char last_error[100];
    MachineInfo last_info;          // Cache last successful read
    bool info_valid;                // Whether cached info is valid
    bool enabled;                   // Whether this machine is enabled
} MachineHandle;

// Connection pool for multiple machines
typedef struct {
    MachineHandle machines[MAX_MACHINES];
    int machine_count;
    time_t pool_created;
    int total_connections;
    int successful_operations;
    int failed_operations;
    bool initialized;
} ConnectionPool;

// Multi-machine information structure
typedef struct {
    int machine_count;
    MachineInfo machines[MAX_MACHINES];
    time_t collection_time;
    int successful_reads;
    int failed_reads;
} MultiMachineInfo;

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
    FOCAS_ALARM_READ_FAILED = -8,
    FOCAS_POOL_FULL = -9,
    FOCAS_MACHINE_NOT_FOUND = -10,
    FOCAS_INVALID_CONFIG = -11
} FocasResult;

// Output formats
typedef enum {
    OUTPUT_CONSOLE = 0,
    OUTPUT_JSON = 1,
    OUTPUT_CSV = 2
} OutputFormat;

// Function declarations

// Configuration management
int read_config(int argc, char *argv[], Config *conf);
int load_machines_from_file(const char *filename, ConnectionPool *pool);
int parse_machine_spec(const char *spec, char *name, char *ip, int *port);

// Connection pool management
FocasResult connection_pool_init(ConnectionPool *pool);
FocasResult connection_pool_add_machine(ConnectionPool *pool, 
                                      const char *name, const char *ip, int port);
FocasResult connection_pool_connect_all(ConnectionPool *pool);
FocasResult connection_pool_disconnect_all(ConnectionPool *pool);
FocasResult connection_pool_read_all_info(ConnectionPool *pool, MultiMachineInfo *multi_info);
void connection_pool_print_status(const ConnectionPool *pool);
void connection_pool_cleanup(ConnectionPool *pool);

// Machine information reading
FocasResult read_machine_info(const char *ip, int port, MachineInfo *info);
FocasResult read_complete_machine_info(Config *conf, MachineInfo *info);

// Output formatting
void print_machine_info(const MachineInfo *info, const char *machine_name);
void print_selective_machine_info(const MachineInfo *info, const char *machine_name, const char *info_type);
void print_multi_machine_info(const MultiMachineInfo *multi_info, const char *info_type, OutputFormat format);
void print_machine_info_json(const MachineInfo *info, const char *machine_name);
void print_machine_info_csv(const MachineInfo *info, const char *machine_name, bool header);

// Monitoring
int monitor_machines(ConnectionPool *pool, Config *conf);

// Utility functions
const char* focas_result_to_string(FocasResult result);
const char* connection_state_to_string(ConnectionState state);
void show_usage(const char *program_name);
void show_version(void);
OutputFormat parse_output_format(const char *format_str);

#endif // FOCAS_MONITOR_H
