#include "focasmonitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

// Include the official FANUC header
#include "fwlib32.h"

// Convert FOCAS error codes to human-readable messages
const char* focas_error_to_string(short error_code) {
  switch (error_code) {
    case EW_OK:
      return "No error";
    case EW_FUNC:
      return "Function not supported";
    case EW_LENGTH:
      return "Data length error";
    case EW_NUMBER:
      return "Data number error";
    case EW_ATTRIB:
      return "Data attribute error";
    case EW_DATA:
      return "Data error";
    case EW_NOOPT:
      return "Option not available";
    case EW_PROT:
      return "Write protection error";
    case EW_OVRFLOW:
      return "Memory overflow";
    case EW_PARAM:
      return "Parameter error";
    case EW_BUFFER:
      return "Buffer error";
    case EW_PATH:
      return "Path error";
    case EW_MODE:
      return "Mode error";
    case EW_REJECT:
      return "Execution rejected";
    case EW_DTSRVR:
      return "Data server error";
    case EW_ALARM:
      return "Alarm state";
    case EW_STOP:
      return "Not running";
    case EW_PASSWD:
      return "Password error";
    case EW_PROTOCOL:
      return "Protocol error";
    case EW_SOCKET:
      return "Socket communication error";
    case EW_NODLL:
      return "DLL not found";
    case EW_BUS:
      return "Bus error";
    case EW_SYSTEM2:
      return "System error";
    case EW_HSSB:
      return "HSSB communication error";
    case EW_HANDLE:
      return "Invalid handle";
    case EW_VERSION:
      return "Version mismatch";
    case EW_UNEXP:
      return "Unexpected error";
    case EW_SYSTEM:
      return "System error";
    case EW_PARITY:
      return "Parity error";
    case EW_MMCSYS:
      return "MMC system error";
    case EW_RESET:
      return "Reset required";
    case EW_BUSY:
      return "Busy";
    default:
      return "Unknown FOCAS error";
  }
}

// Get detailed error information for connection failures
const char* get_connection_error_details(short error_code) {
  switch (error_code) {
    case EW_SOCKET:
      return "Network connection failed\n"
             "  > Check if machine IP address is correct and reachable (try: ping <ip>)\n"
             "  > Verify port 8193 is open on the machine (try: telnet <ip> 8193)\n"
             "  > Ensure machine is powered on and network cable is connected\n"
             "  > Check if firewall is blocking the connection";
    case EW_PROTOCOL:
      return "FOCAS protocol error\n"
             "  > Machine may not support FOCAS ethernet communication\n"
             "  > Verify correct port number (standard FOCAS port is 8193)\n"
             "  > Check machine ethernet board configuration\n"
             "  > Ensure FOCAS ethernet option is installed on the machine";
    case EW_PASSWD:
      return "Authentication failed\n"
             "  > Check if machine requires password for FOCAS connections\n"
             "  > Verify password settings in machine parameters\n"
             "  > Some machines require specific user authentication";
    case EW_NODLL:
      return "FOCAS library (DLL) not found or incompatible\n"
             "  > Ensure all required FANUC DLLs are in the same directory\n"
             "  > Check if DLL version matches your machine's FOCAS version\n"
             "  > Try running as administrator if DLL loading fails";
    case EW_VERSION:
      return "FOCAS version mismatch\n"
             "  > Machine FOCAS version incompatible with library version\n"
             "  > Try using different FOCAS DLL version\n"
             "  > Check machine's FOCAS specification and requirements";
    case EW_HANDLE:
      return "Invalid connection handle\n"
             "  > Previous connection may have been unexpectedly closed\n"
             "  > Machine may have reset or been power cycled\n"
             "  > Network connection was interrupted";
    case EW_BUSY:
      return "Machine is busy or overloaded\n"
             "  > Too many simultaneous FOCAS connections to machine\n"
             "  > Wait and retry connection later\n"
             "  > Check if other applications are connected to the machine";
    case EW_ALARM:
      return "Machine is in alarm state\n"
             "  > Resolve all active alarms on the machine before connecting\n"
             "  > Check machine display for alarm messages\n"
             "  > Some alarms prevent external communication";
    case EW_NOOPT:
      return "FOCAS option not enabled\n"
             "  > Ethernet FOCAS option not purchased/enabled on machine\n"
             "  > Check machine option list and parameter settings\n"
             "  > Contact FANUC dealer to enable FOCAS ethernet option";
    case EW_FUNC:
      return "Function not supported\n"
             "  > This FOCAS function is not available on this machine model\n"
             "  > Check machine's FOCAS capability and supported functions";
    case EW_MODE:
      return "Machine mode error\n"
             "  > Machine may be in wrong mode for FOCAS communication\n"
             "  > Check if machine is in appropriate operating mode\n"
             "  > Some modes restrict external communication";
    default:
      return focas_error_to_string(error_code);
  }
}

// Ping check utility function for network diagnostics
int ping_host(const char* ip_address) {
#ifdef _WIN32
  char command[256];
  snprintf(command, sizeof(command), "ping -n 1 -w 1000 %s >nul 2>&1", ip_address);
  return system(command);
#else
  char command[256];
  snprintf(command, sizeof(command), "ping -c 1 -W 1 %s >/dev/null 2>&1", ip_address);
  return system(command);
#endif
}

// Perform basic network diagnostics
void perform_network_diagnostics(const char* machine_name, const char* ip, int port, bool verbose) {
  if (verbose) {
    printf("  > Running network diagnostics for %s...\n", machine_name);
    
    // Test ping connectivity
    printf("    Testing ping connectivity to %s...", ip);
    fflush(stdout);
    
    int ping_result = ping_host(ip);
    if (ping_result == 0) {
      printf(" [OK] Host is reachable\n");
    } else {
      printf(" [FAIL] Host unreachable\n");
      printf("    This indicates a basic network connectivity problem:\n");
      printf("      * Check if the IP address %s is correct\n", ip);
      printf("      * Verify network cables and switch connections\n");
      printf("      * Check if the machine is powered on\n");
      printf("      * Ensure you're on the correct network segment/VLAN\n");
      return; // Skip port test if ping fails
    }
    
    // Test port connectivity (simplified check)
#ifdef _WIN32
    printf("    Testing port %d connectivity...", port);
    fflush(stdout);
    
    char telnet_cmd[256];
    snprintf(telnet_cmd, sizeof(telnet_cmd), 
             "powershell -Command \"try { $socket = New-Object System.Net.Sockets.TcpClient; $socket.Connect('%s', %d); $socket.Close(); exit 0 } catch { exit 1 }\" >nul 2>&1", 
             ip, port);
    
    int port_result = system(telnet_cmd);
    if (port_result == 0) {
      printf(" [OK] Port %d is open\n", port);
    } else {
      printf(" [FAIL] Port %d appears closed or filtered\n", port);
      printf("    This suggests:\n");
      printf("      * FOCAS service may not be running on the machine\n");
      printf("      * Port %d is blocked by firewall\n", port);
      printf("      * Machine is not configured for FOCAS ethernet communication\n");
    }
#else
    printf("    Port connectivity test not available on this platform\n");
#endif
    
    printf("\n");
  }
}

FocasResult connection_pool_init(ConnectionPool *pool) {
  if (!pool)
    return FOCAS_CONNECTION_FAILED;

  memset(pool, 0, sizeof(ConnectionPool));
  pool->pool_created = time(NULL);
  pool->initialized = true;

  return FOCAS_OK;
}

FocasResult connection_pool_add_machine(ConnectionPool *pool, const char *name,
                                        const char *ip, int port) {
  if (!pool || !ip || !name)
    return FOCAS_CONNECTION_FAILED;

  if (!pool->initialized) {
    return FOCAS_INVALID_CONFIG;
  }

  if (pool->machine_count >= MAX_MACHINES) {
    return FOCAS_POOL_FULL;
  }

  MachineHandle *machine = &pool->machines[pool->machine_count];

  // Initialize machine handle
  strncpy(machine->ip, ip, sizeof(machine->ip) - 1);
  machine->ip[sizeof(machine->ip) - 1] = '\0';
  machine->port = port;
  strncpy(machine->friendly_name, name, sizeof(machine->friendly_name) - 1);
  machine->friendly_name[sizeof(machine->friendly_name) - 1] = '\0';
  machine->handle = 0;
  machine->state = CONN_DISCONNECTED;
  machine->retry_count = 0;
  machine->info_valid = false;
  machine->enabled = true;
  strcpy(machine->last_error, "Not connected");

  pool->machine_count++;

  return FOCAS_OK;
}

FocasResult connection_pool_connect_machine(ConnectionPool *pool,
                                            int machine_id, bool diagnose) {
  if (!pool || machine_id < 0 || machine_id >= pool->machine_count) {
    return FOCAS_CONNECTION_FAILED;
  }

  MachineHandle *machine = &pool->machines[machine_id];

  if (!machine->enabled) {
    return FOCAS_OK; // Skip disabled machines
  }

  if (machine->state == CONN_CONNECTED) {
    return FOCAS_OK; // Already connected
  }

  machine->state = CONN_CONNECTING;

  // Attempt connection using FOCAS
  printf("> Connecting to %s at %s:%d (timeout: %ds)...\n", 
         machine->friendly_name, machine->ip, machine->port, CONNECTION_TIMEOUT);
  short result = cnc_allclibhndl3(machine->ip, machine->port,
                                  CONNECTION_TIMEOUT, &machine->handle);

  if (result == EW_OK) {
    machine->state = CONN_CONNECTED;
    machine->connect_time = time(NULL);
    machine->last_activity = time(NULL);
    machine->retry_count = 0;
    strcpy(machine->last_error, "Connected successfully");
    pool->total_connections++;
    printf("[OK] Successfully connected to %s (handle: %d)\n", 
           machine->friendly_name, machine->handle);

    return FOCAS_OK;
  } else {
    machine->state = CONN_ERROR;
    machine->retry_count++;
    
    // Use detailed error mapping
    const char* error_msg = get_connection_error_details(result);
    snprintf(machine->last_error, sizeof(machine->last_error),
             "FOCAS error %d: %s", result, focas_error_to_string(result));
    
    printf("[FAIL] Connection to %s FAILED (FOCAS error %d: %s)\n", 
           machine->friendly_name, result, focas_error_to_string(result));
    
    // Perform network diagnostics for socket errors
    if (result == EW_SOCKET && diagnose) {
      perform_network_diagnostics(machine->friendly_name, machine->ip, machine->port, true);
    } else {
      printf("  Troubleshooting steps:\n");
      
      // Print the detailed error message with proper indentation
      char* msg_copy = strdup(error_msg);
      char* line = strtok(msg_copy, "\n");
      while (line != NULL) {
        printf("  %s\n", line);
        line = strtok(NULL, "\n");
      }
      free(msg_copy);
      printf("\n");
    }

    return FOCAS_CONNECTION_FAILED;
  }
}

FocasResult connection_pool_disconnect_machine(ConnectionPool *pool,
                                               int machine_id) {
  if (!pool || machine_id < 0 || machine_id >= pool->machine_count) {
    return FOCAS_CONNECTION_FAILED;
  }

  MachineHandle *machine = &pool->machines[machine_id];

  if (machine->state == CONN_CONNECTED && machine->handle != 0) {
    cnc_freelibhndl(machine->handle);
    machine->state = CONN_DISCONNECTED;
    machine->handle = 0;
    strcpy(machine->last_error, "Disconnected");
  }

  return FOCAS_OK;
}

FocasResult connection_pool_connect_all(ConnectionPool *pool, bool diagnose) {
  if (!pool || !pool->initialized) {
    return FOCAS_INVALID_CONFIG;
  }

  int successful = 0;
  int failed = 0;

  for (int i = 0; i < pool->machine_count; i++) {
    FocasResult result = connection_pool_connect_machine(pool, i, diagnose);
    if (result == FOCAS_OK) {
      successful++;
    } else {
      failed++;
    }
  }

  // Print connection summary
  if (failed > 0) {
    printf("===============================================================\n");
    printf("CONNECTION SUMMARY: %d successful, %d failed\n", successful, failed);
    if (failed == pool->machine_count) {
      printf("WARNING: All machines failed to connect. Common solutions:\n");
      printf("  * Verify machine IP addresses are correct\n");
      printf("  * Check network connectivity (ping machines)\n");
      printf("  * Ensure machines are powered on\n");
      printf("  * Verify FOCAS ethernet option is enabled\n");
      printf("  * Check firewall settings\n");
      printf("  TIP: Use --diagnose flag for automatic network diagnostics\n");
    }
    printf("===============================================================\n\n");
  }

  return (failed == 0) ? FOCAS_OK : FOCAS_CONNECTION_FAILED;
}

FocasResult connection_pool_disconnect_all(ConnectionPool *pool) {
  if (!pool || !pool->initialized) {
    return FOCAS_INVALID_CONFIG;
  }

  for (int i = 0; i < pool->machine_count; i++) {
    connection_pool_disconnect_machine(pool, i);
  }

  return FOCAS_OK;
}

FocasResult read_machine_info_from_handle(unsigned short handle, MachineInfo *info) {
  if (handle == 0 || !info) {
    return FOCAS_CONNECTION_FAILED;
  }

  // Initialize info structure
  memset(info, 0, sizeof(MachineInfo));
  info->last_updated = time(NULL);

  // Read machine ID
  unsigned long cncid[4];
  if (cnc_rdcncid(handle, cncid) == EW_OK) {
    snprintf(info->machine_id, sizeof(info->machine_id),
             "%08lx-%08lx-%08lx-%08lx", cncid[0], cncid[1], cncid[2], cncid[3]);
  } else {
    strcpy(info->machine_id, "UNKNOWN");
  }

  // Read current program
  ODBPRO prgnum;
  if (cnc_rdprgnum(handle, &prgnum) == EW_OK) {
    snprintf(info->program_name, sizeof(info->program_name), "O%04d",
             prgnum.data);
    info->program_number = (int)prgnum.data;
  } else {
    strcpy(info->program_name, "UNKNOWN");
    info->program_number = 0;
  }

  // Read machine status
  ODBST status;
  if (cnc_statinfo(handle, &status) == EW_OK) {
    switch (status.run) {
    case 0:
      strcpy(info->status, "STOPPED");
      break;
    case 1:
      strcpy(info->status, "RUNNING");
      break;
    case 2:
      strcpy(info->status, "PAUSED");
      break;
    case 3:
      strcpy(info->status, "ALARM");
      break;
    default:
      snprintf(info->status, sizeof(info->status), "UNKNOWN(%d)", status.run);
      break;
    }

    // Add motion status if available
    if (status.motion == 1) {
      strcat(info->status, " (MOVING)");
    }
  } else {
    strcpy(info->status, "UNKNOWN");
  }

  // Read sequence number
  ODBSEQ seq_info;
  if (cnc_rdseqnum(handle, &seq_info) == EW_OK) {
    info->sequence_number = seq_info.data;
    info->program_line = (int)seq_info.data;
  } else {
    info->sequence_number = 0;
    info->program_line = 0;
  }

  // Read position information (simplified - first axis only for now)
  ODBPOS pos_data;
  short num_axes = 3;
  if (cnc_rdposition(handle, 0, &num_axes, &pos_data) == EW_OK) {
    // Position data is in the 'data' field, scaled by decimal places
    double scale = 1.0;
    if (pos_data.abs.dec > 0) {
      for (int i = 0; i < pos_data.abs.dec; i++) {
        scale /= 10.0;
      }
    }

    info->position.x_abs = pos_data.abs.data * scale;
    info->position.x_rel = pos_data.rel.data * scale;

    // Y and Z would require additional calls with different axis numbers
    info->position.y_abs = 0.0;
    info->position.z_abs = 0.0;
    info->position.y_rel = 0.0;
    info->position.z_rel = 0.0;
  } else {
    memset(&info->position, 0, sizeof(PositionInfo));
  }

  // Read speed information
  ODBSPEED speed_data;
  if (cnc_rdspeed(handle, 0, &speed_data) == EW_OK) {
    info->speed.feed_rate = speed_data.actf.data;
    info->speed.spindle_speed = speed_data.acts.data;
  } else {
    memset(&info->speed, 0, sizeof(SpeedInfo));
  }

  // Read alarm information
  ODBALM alarm_data;
  if (cnc_alarm(handle, &alarm_data) == EW_OK) {
    info->alarm.alarm_status = alarm_data.data;
    info->alarm.has_alarm = (alarm_data.data != 0) ? 1 : 0;
  } else {
    memset(&info->alarm, 0, sizeof(AlarmInfo));
  }

  return FOCAS_OK;
}

FocasResult read_machine_info(const char *ip, int port, MachineInfo *info) {
  unsigned short libh;
  FocasResult result = FOCAS_OK;

  // Initialize info structure
  memset(info, 0, sizeof(MachineInfo));
  info->last_updated = time(NULL);

  // Connect to machine
  short connect_result = cnc_allclibhndl3(ip, port, CONNECTION_TIMEOUT, &libh);
  if (connect_result != EW_OK) {
    printf("Connection to %s:%d failed: %s (FOCAS error %d)\n", 
           ip, port, get_connection_error_details(connect_result), connect_result);
    return FOCAS_CONNECTION_FAILED;
  }

  // Use the optimized function with the temporary handle
  result = read_machine_info_from_handle(libh, info);

  // Cleanup
  cnc_freelibhndl(libh);

  return result;
}

FocasResult connection_pool_read_all_info(ConnectionPool *pool,
                                          MultiMachineInfo *multi_info) {
  if (!pool || !multi_info || !pool->initialized) {
    return FOCAS_INVALID_CONFIG;
  }

  memset(multi_info, 0, sizeof(MultiMachineInfo));
  multi_info->collection_time = time(NULL);

  for (int i = 0; i < pool->machine_count; i++) {
    MachineHandle *machine = &pool->machines[i];

    if (!machine->enabled) {
      continue;
    }

    // Copy machine info
    MachineInfo *info = &multi_info->machines[multi_info->machine_count];
    FocasResult result = FOCAS_CONNECTION_FAILED;

    // Try to use persistent connection first
    if (machine->state == CONN_CONNECTED && machine->handle != 0) {
      result = read_machine_info_from_handle(machine->handle, info);
      if (result == FOCAS_OK) {
        machine->last_activity = time(NULL);
      } else {
        // Connection might be stale, try to reconnect
        printf("WARNING: Persistent connection to %s failed, attempting automatic reconnection...\n", 
               machine->friendly_name);
        connection_pool_disconnect_machine(pool, i);
        connection_pool_connect_machine(pool, i, false);
        
        // Retry with new connection
        if (machine->state == CONN_CONNECTED && machine->handle != 0) {
          result = read_machine_info_from_handle(machine->handle, info);
          if (result == FOCAS_OK) {
            machine->last_activity = time(NULL);
          }
        }
      }
    } else {
      // Not connected, try to connect and read
      if (connection_pool_connect_machine(pool, i, false) == FOCAS_OK) {
        if (machine->handle != 0) {
          result = read_machine_info_from_handle(machine->handle, info);
          if (result == FOCAS_OK) {
            machine->last_activity = time(NULL);
          }
        }
      }
    }

    if (result == FOCAS_OK) {
      machine->last_info = *info;
      machine->info_valid = true;
      multi_info->successful_reads++;
      pool->successful_operations++;
    } else {
      // Use cached info if available
      if (machine->info_valid) {
        *info = machine->last_info;
        multi_info->successful_reads++;
        printf("Using cached data for %s\n", machine->friendly_name);
      } else {
        multi_info->failed_reads++;
        pool->failed_operations++;
        printf("WARNING: Failed to read from %s: %s\n", 
               machine->friendly_name, machine->last_error);
        printf("  No cached data available - machine data will be missing from this cycle\n");
        continue; // Skip this machine
      }
    }

    multi_info->machine_count++;
  }

  return (multi_info->failed_reads == 0) ? FOCAS_OK : FOCAS_CONNECTION_FAILED;
}

void connection_pool_print_status(const ConnectionPool *pool) {
  if (!pool || !pool->initialized) {
    printf("Connection pool not initialized\n");
    return;
  }

  printf("=== Connection Pool Status ===\n");
  printf("Machines configured: %d / %d\n", pool->machine_count, MAX_MACHINES);
  printf("Total connections: %d\n", pool->total_connections);
  printf("Successful operations: %d\n", pool->successful_operations);
  printf("Failed operations: %d\n", pool->failed_operations);

  time_t now = time(NULL);
  printf("Pool created: %ld seconds ago\n", now - pool->pool_created);

  printf("\n--- Machine Details ---\n");
  for (int i = 0; i < pool->machine_count; i++) {
    const MachineHandle *machine = &pool->machines[i];
    printf("[%d] %s (%s:%d)\n", i, machine->friendly_name, machine->ip,
           machine->port);
    printf("    State: %s\n", connection_state_to_string(machine->state));
    printf("    Enabled: %s\n", machine->enabled ? "Yes" : "No");
    printf("    Retry count: %d\n", machine->retry_count);
    printf("    Last error: %s\n", machine->last_error);
    if (machine->state == CONN_CONNECTED) {
      printf("    Connected for: %ld seconds\n", now - machine->connect_time);
      printf("    Last activity: %ld seconds ago\n",
             now - machine->last_activity);
    }
    printf("    Cached info valid: %s\n", machine->info_valid ? "Yes" : "No");
    printf("\n");
  }
}

void connection_pool_cleanup(ConnectionPool *pool) {
  if (pool && pool->initialized) {
    connection_pool_disconnect_all(pool);
    memset(pool, 0, sizeof(ConnectionPool));
  }
}
