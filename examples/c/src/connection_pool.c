#include "connection_pool.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

// Include the official FANUC header
#include "fwlib32.h"

FocasResult init_connection_pool(ConnectionPool *pool) {
    if (!pool) return FOCAS_CONNECTION_FAILED;
    
    memset(pool, 0, sizeof(ConnectionPool));
    pool->pool_created = time(NULL);
    
    printf("🔧 Initialized connection pool for up to %d machines\n", MAX_MACHINES);
    return FOCAS_OK;
}

FocasResult add_machine(ConnectionPool *pool, const char *ip, int port, const char *name) {
    if (!pool || !ip || !name) return FOCAS_CONNECTION_FAILED;
    
    if (pool->machine_count >= MAX_MACHINES) {
        printf("❌ Connection pool is full (max %d machines)\n", MAX_MACHINES);
        return FOCAS_CONNECTION_FAILED;
    }
    
    MachineHandle *machine = &pool->machines[pool->machine_count];
    
    // Initialize machine handle
    strncpy(machine->ip, ip, sizeof(machine->ip) - 1);
    machine->port = port;
    strncpy(machine->friendly_name, name, sizeof(machine->friendly_name) - 1);
    machine->handle = 0;
    machine->state = CONN_DISCONNECTED;
    machine->retry_count = 0;
    machine->info_valid = 0;
    strcpy(machine->last_error, "Not connected");
    
    pool->machine_count++;
    
    printf("➕ Added machine '%s' at %s:%d (ID: %d)\n", 
           name, ip, port, pool->machine_count - 1);
    
    return FOCAS_OK;
}

FocasResult connect_machine(ConnectionPool *pool, int machine_id) {
    if (!pool || machine_id < 0 || machine_id >= pool->machine_count) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    MachineHandle *machine = &pool->machines[machine_id];
    
    if (machine->state == CONN_CONNECTED) {
        // Already connected
        return FOCAS_OK;
    }
    
    printf("🔌 Connecting to %s (%s:%d)...\n", 
           machine->friendly_name, machine->ip, machine->port);
    
    machine->state = CONN_CONNECTING;
    
    // Attempt connection using FOCAS
    short result = cnc_allclibhndl3(machine->ip, machine->port, CONNECTION_TIMEOUT, &machine->handle);
    
    if (result == EW_OK) {
        machine->state = CONN_CONNECTED;
        machine->connect_time = time(NULL);
        machine->last_activity = time(NULL);
        machine->retry_count = 0;
        strcpy(machine->last_error, "Connected successfully");
        pool->total_connections++;
        
        printf("✅ Connected to %s (handle: %d)\n", machine->friendly_name, machine->handle);
        return FOCAS_OK;
    } else {
        machine->state = CONN_ERROR;
        machine->retry_count++;
        snprintf(machine->last_error, sizeof(machine->last_error), 
                "Connection failed: FOCAS error %d", result);
        
        printf("❌ Failed to connect to %s: %s\n", machine->friendly_name, machine->last_error);
        return FOCAS_CONNECTION_FAILED;
    }
}

FocasResult disconnect_machine(ConnectionPool *pool, int machine_id) {
    if (!pool || machine_id < 0 || machine_id >= pool->machine_count) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    MachineHandle *machine = &pool->machines[machine_id];
    
    if (machine->state == CONN_CONNECTED && machine->handle != 0) {
        cnc_freelibhndl(machine->handle);
        printf("🔌 Disconnected from %s\n", machine->friendly_name);
    }
    
    machine->handle = 0;
    machine->state = CONN_DISCONNECTED;
    machine->info_valid = 0;
    strcpy(machine->last_error, "Disconnected");
    
    return FOCAS_OK;
}

FocasResult disconnect_all_machines(ConnectionPool *pool) {
    if (!pool) return FOCAS_CONNECTION_FAILED;
    
    printf("🔌 Disconnecting all machines...\n");
    
    for (int i = 0; i < pool->machine_count; i++) {
        disconnect_machine(pool, i);
    }
    
    printf("✅ All machines disconnected\n");
    return FOCAS_OK;
}

MachineHandle* get_machine_handle(ConnectionPool *pool, int machine_id) {
    if (!pool || machine_id < 0 || machine_id >= pool->machine_count) {
        return NULL;
    }
    
    return &pool->machines[machine_id];
}

FocasResult read_machine_by_id(ConnectionPool *pool, int machine_id, MachineInfo *info) {
    if (!pool || !info || machine_id < 0 || machine_id >= pool->machine_count) {
        return FOCAS_CONNECTION_FAILED;
    }
    
    MachineHandle *machine = &pool->machines[machine_id];
    
    // Check if we need to connect
    if (machine->state != CONN_CONNECTED) {
        FocasResult connect_result = connect_machine(pool, machine_id);
        if (connect_result != FOCAS_OK) {
            // Return cached info if available
            if (machine->info_valid) {
                memcpy(info, &machine->last_info, sizeof(MachineInfo));
                return FOCAS_OK; // Partial success with cached data
            }
            return connect_result;
        }
    }
    
    // Create a temporary config for this machine
    Config temp_config;
    strncpy(temp_config.ip, machine->ip, sizeof(temp_config.ip));
    temp_config.port = machine->port;
    strncpy(temp_config.info_type, "all", sizeof(temp_config.info_type));
    temp_config.verbose = 0;
    
    // Read machine information using existing function
    FocasResult result = read_complete_machine_info(&temp_config, info);
    
    machine->last_activity = time(NULL);
    
    if (result == FOCAS_OK) {
        // Cache successful read
        memcpy(&machine->last_info, info, sizeof(MachineInfo));
        machine->info_valid = 1;
        pool->successful_operations++;
    } else {
        pool->failed_operations++;
        
        // Handle specific errors
        if (result == FOCAS_CONNECTION_FAILED) {
            machine->state = CONN_ERROR;
            strcpy(machine->last_error, "Lost connection during read");
        }
    }
    
    return result;
}

// Get current time in milliseconds
double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

FocasResult read_all_machines_info(ConnectionPool *pool, MultiMachineInfo *info) {
    if (!pool || !info) return FOCAS_CONNECTION_FAILED;
    
    double start_time = get_time_ms();
    
    // Initialize multi-machine info
    memset(info, 0, sizeof(MultiMachineInfo));
    info->machine_count = pool->machine_count;
    info->collection_time = time(NULL);
    
    printf("📊 Reading information from %d machines...\n", pool->machine_count);
    
    // Read from each machine sequentially
    for (int i = 0; i < pool->machine_count; i++) {
        MachineHandle *machine = &pool->machines[i];
        
        printf("  📡 Reading from %s...", machine->friendly_name);
        
        FocasResult result = read_machine_by_id(pool, i, &info->machines[i]);
        
        // Copy machine metadata
        strncpy(info->machine_names[i], machine->friendly_name, 
                sizeof(info->machine_names[i]) - 1);
        info->states[i] = machine->state;
        
        if (result == FOCAS_OK) {
            info->successful_reads++;
            printf(" ✅\n");
        } else {
            info->failed_reads++;
            printf(" ❌ (%s)\n", focas_result_to_string(result));
        }
    }
    
    double end_time = get_time_ms();
    info->collection_duration_ms = end_time - start_time;
    
    printf("📊 Collection complete: %d successful, %d failed (%.1fms)\n", 
           info->successful_reads, info->failed_reads, info->collection_duration_ms);
    
    return (info->successful_reads > 0) ? FOCAS_OK : FOCAS_CONNECTION_FAILED;
}

const char* connection_state_to_string(ConnectionState state) {
    switch (state) {
        case CONN_DISCONNECTED: return "DISCONNECTED";
        case CONN_CONNECTING: return "CONNECTING";
        case CONN_CONNECTED: return "CONNECTED";
        case CONN_ERROR: return "ERROR";
        case CONN_BUSY: return "BUSY";
        default: return "UNKNOWN";
    }
}

void print_connection_pool_status(const ConnectionPool *pool) {
    if (!pool) return;
    
    time_t now = time(NULL);
    double uptime = difftime(now, pool->pool_created);
    
    printf("\n=== Connection Pool Status ===\n");
    printf("Pool uptime: %.0f seconds\n", uptime);
    printf("Total machines: %d\n", pool->machine_count);
    printf("Total connections made: %d\n", pool->total_connections);
    printf("Successful operations: %d\n", pool->successful_operations);
    printf("Failed operations: %d\n", pool->failed_operations);
    
    if (pool->machine_count > 0) {
        printf("\nMachine Details:\n");
        for (int i = 0; i < pool->machine_count; i++) {
            const MachineHandle *machine = &pool->machines[i];
            printf("  [%d] %s (%s:%d)\n", i, machine->friendly_name, machine->ip, machine->port);
            printf("      State: %s\n", connection_state_to_string(machine->state));
            printf("      Handle: %d\n", machine->handle);
            printf("      Retries: %d\n", machine->retry_count);
            if (machine->state == CONN_CONNECTED) {
                double idle_time = difftime(now, machine->last_activity);
                printf("      Idle: %.0f seconds\n", idle_time);
            }
            printf("      Status: %s\n", machine->last_error);
            printf("\n");
        }
    }
    printf("===============================\n");
}

void print_multi_machine_info(const MultiMachineInfo *info, const char *display_mode) {
    if (!info || !display_mode) return;
    
    printf("\n=== Multi-Machine FANUC Information ===\n");
    printf("Collection Time: %s", ctime(&info->collection_time));
    printf("Duration: %.1fms\n", info->collection_duration_ms);
    printf("Success Rate: %d/%d (%.1f%%)\n", 
           info->successful_reads, info->machine_count,
           info->machine_count > 0 ? (100.0 * info->successful_reads / info->machine_count) : 0.0);
    printf("========================================\n\n");
    
    for (int i = 0; i < info->machine_count; i++) {
        printf("🏭 Machine [%d]: %s\n", i, info->machine_names[i]);
        printf("   Status: %s\n", connection_state_to_string(info->states[i]));
        
        if (info->states[i] == CONN_CONNECTED || info->states[i] == CONN_ERROR) {
            const MachineInfo *machine_info = &info->machines[i];
            
            if (strcmp(display_mode, "basic") == 0) {
                printf("   Machine ID: %s\n", machine_info->machine_id);
                printf("   Status: %s\n", machine_info->status);
            } else if (strcmp(display_mode, "program") == 0) {
                printf("   Program: %s", machine_info->program_name);
                if (machine_info->program_number > 0) {
                    printf(" (Number: %d)", machine_info->program_number);
                }
                printf("\n");
                if (machine_info->sequence_number > 0) {
                    printf("   Sequence: N%ld\n", machine_info->sequence_number);
                }
            } else if (strcmp(display_mode, "alarm") == 0) {
                if (machine_info->alarm.has_alarm) {
                    printf("   ⚠️ ALARM ACTIVE (Code: %d)\n", machine_info->alarm.alarm_status);
                } else {
                    printf("   ✅ No Alarms\n");
                }
            } else {
                // Default: show summary info
                printf("   Machine ID: %s\n", machine_info->machine_id);
                printf("   Status: %s\n", machine_info->status);
                printf("   Program: %s\n", machine_info->program_name);
                if (machine_info->alarm.has_alarm) {
                    printf("   ⚠️ ALARM: Code %d\n", machine_info->alarm.alarm_status);
                } else {
                    printf("   ✅ No Alarms\n");
                }
            }
        } else {
            printf("   ❌ No data available\n");
        }
        
        printf("\n");
    }
    
    printf("========================================\n");
}

// Machine list configuration support
FocasResult load_machine_list_from_config(const char *config_file, ConnectionPool *pool) {
    if (!config_file || !pool) return FOCAS_CONNECTION_FAILED;
    
    // For now, implement a simple text-based config format
    // Format: name,ip,port (one per line)
    FILE *file = fopen(config_file, "r");
    if (!file) {
        printf("⚠️ Could not open machine list config: %s\n", config_file);
        return FOCAS_CONNECTION_FAILED;
    }
    
    char line[256];
    int machines_loaded = 0;
    
    printf("📄 Loading machine list from %s...\n", config_file);
    
    while (fgets(line, sizeof(line), file) && machines_loaded < MAX_MACHINES) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        // Parse: name,ip,port
        char name[50], ip[100];
        int port;
        
        if (sscanf(line, "%49[^,],%99[^,],%d", name, ip, &port) == 3) {
            if (add_machine(pool, ip, port, name) == FOCAS_OK) {
                machines_loaded++;
            }
        } else {
            printf("⚠️ Invalid line format: %s", line);
        }
    }
    
    fclose(file);
    
    printf("✅ Loaded %d machines from config file\n", machines_loaded);
    return (machines_loaded > 0) ? FOCAS_OK : FOCAS_CONNECTION_FAILED;
}

FocasResult save_machine_list_to_config(const char *config_file, const ConnectionPool *pool) {
    if (!config_file || !pool) return FOCAS_CONNECTION_FAILED;
    
    FILE *file = fopen(config_file, "w");
    if (!file) {
        printf("❌ Could not create machine list config: %s\n", config_file);
        return FOCAS_CONNECTION_FAILED;
    }
    
    fprintf(file, "# Machine List Configuration\n");
    fprintf(file, "# Format: name,ip,port\n");
    fprintf(file, "# Generated on %s\n", ctime(&pool->pool_created));
    
    for (int i = 0; i < pool->machine_count; i++) {
        const MachineHandle *machine = &pool->machines[i];
        fprintf(file, "%s,%s,%d\n", machine->friendly_name, machine->ip, machine->port);
    }
    
    fclose(file);
    printf("💾 Saved %d machines to config file: %s\n", pool->machine_count, config_file);
    return FOCAS_OK;
}
