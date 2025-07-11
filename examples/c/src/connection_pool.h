#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include "config.h"
#include "machine_info.h"
#include <time.h>

// Maximum number of machines per connection pool
#define MAX_MACHINES 10

// Connection timeout in seconds
#define CONNECTION_TIMEOUT 10

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
  MachineInfo last_info; // Cache last successful read
  int info_valid;        // Whether cached info is valid
} MachineHandle;

// Connection pool for multiple machines
typedef struct {
  MachineHandle machines[MAX_MACHINES];
  int machine_count;
  time_t pool_created;
  int total_connections;
  int successful_operations;
  int failed_operations;
} ConnectionPool;

// Multi-machine information structure
typedef struct {
  int machine_count;
  MachineInfo machines[MAX_MACHINES];
  char machine_names[MAX_MACHINES][50];
  ConnectionState states[MAX_MACHINES];
  time_t collection_time;
  int successful_reads;
  int failed_reads;
  double collection_duration_ms;
} MultiMachineInfo;

// Connection pool management functions
FocasResult init_connection_pool(ConnectionPool *pool);
FocasResult add_machine(ConnectionPool *pool, const char *ip, int port,
                        const char *name);
FocasResult connect_machine(ConnectionPool *pool, int machine_id);
FocasResult disconnect_machine(ConnectionPool *pool, int machine_id);
FocasResult disconnect_all_machines(ConnectionPool *pool);
MachineHandle *get_machine_handle(ConnectionPool *pool, int machine_id);

// Multi-machine operations
FocasResult read_all_machines_info(ConnectionPool *pool,
                                   MultiMachineInfo *info);
FocasResult read_machine_by_id(ConnectionPool *pool, int machine_id,
                               MachineInfo *info);

// Connection health and maintenance
FocasResult check_connection_health(ConnectionPool *pool, int machine_id);
FocasResult reconnect_if_needed(ConnectionPool *pool, int machine_id);
void cleanup_stale_connections(ConnectionPool *pool, int max_idle_seconds);

// Display and utility functions
void print_connection_pool_status(const ConnectionPool *pool);
void print_multi_machine_info(const MultiMachineInfo *info,
                              const char *display_mode);
const char *connection_state_to_string(ConnectionState state);

// Configuration helpers
FocasResult load_machine_list_from_config(const char *config_file,
                                          ConnectionPool *pool);
FocasResult save_machine_list_to_config(const char *config_file,
                                        const ConnectionPool *pool);

#endif // CONNECTION_POOL_H
