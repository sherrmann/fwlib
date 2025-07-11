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
                                            int machine_id) {
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
  short result = cnc_allclibhndl3(machine->ip, machine->port,
                                  CONNECTION_TIMEOUT, &machine->handle);

  if (result == EW_OK) {
    machine->state = CONN_CONNECTED;
    machine->connect_time = time(NULL);
    machine->last_activity = time(NULL);
    machine->retry_count = 0;
    strcpy(machine->last_error, "Connected successfully");
    pool->total_connections++;

    return FOCAS_OK;
  } else {
    machine->state = CONN_ERROR;
    machine->retry_count++;
    snprintf(machine->last_error, sizeof(machine->last_error),
             "Connection failed: FOCAS error %d", result);

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

FocasResult connection_pool_connect_all(ConnectionPool *pool) {
  if (!pool || !pool->initialized) {
    return FOCAS_INVALID_CONFIG;
  }

  int successful = 0;
  int failed = 0;

  for (int i = 0; i < pool->machine_count; i++) {
    FocasResult result = connection_pool_connect_machine(pool, i);
    if (result == FOCAS_OK) {
      successful++;
    } else {
      failed++;
    }
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

FocasResult read_machine_info(const char *ip, int port, MachineInfo *info) {
  unsigned short libh;
  FocasResult result = FOCAS_OK;

  // Initialize info structure
  memset(info, 0, sizeof(MachineInfo));
  info->last_updated = time(NULL);

  // Connect to machine
  short connect_result = cnc_allclibhndl3(ip, port, CONNECTION_TIMEOUT, &libh);
  if (connect_result != EW_OK) {
    return FOCAS_CONNECTION_FAILED;
  }

  // Read machine ID
  unsigned long cncid[4];
  if (cnc_rdcncid(libh, cncid) == EW_OK) {
    snprintf(info->machine_id, sizeof(info->machine_id),
             "%08lx-%08lx-%08lx-%08lx", cncid[0], cncid[1], cncid[2], cncid[3]);
  } else {
    strcpy(info->machine_id, "UNKNOWN");
  }

  // Read current program
  ODBPRO prgnum;
  if (cnc_rdprgnum(libh, &prgnum) == EW_OK) {
    snprintf(info->program_name, sizeof(info->program_name), "O%04d",
             prgnum.data);
    info->program_number = (int)prgnum.data;
  } else {
    strcpy(info->program_name, "UNKNOWN");
    info->program_number = 0;
  }

  // Read machine status
  ODBST status;
  if (cnc_statinfo(libh, &status) == EW_OK) {
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
  if (cnc_rdseqnum(libh, &seq_info) == EW_OK) {
    info->sequence_number = seq_info.data;
    info->program_line = (int)seq_info.data;
  } else {
    info->sequence_number = 0;
    info->program_line = 0;
  }

  // Read position information (simplified - first axis only for now)
  ODBPOS pos_data;
  short num_axes = 3;
  if (cnc_rdposition(libh, 0, &num_axes, &pos_data) == EW_OK) {
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
  if (cnc_rdspeed(libh, 0, &speed_data) == EW_OK) {
    info->speed.feed_rate = speed_data.actf.data;
    info->speed.spindle_speed = speed_data.acts.data;
  } else {
    memset(&info->speed, 0, sizeof(SpeedInfo));
  }

  // Read alarm information
  ODBALM alarm_data;
  if (cnc_alarm(libh, &alarm_data) == EW_OK) {
    info->alarm.alarm_status = alarm_data.data;
    info->alarm.has_alarm = (alarm_data.data != 0) ? 1 : 0;
  } else {
    memset(&info->alarm, 0, sizeof(AlarmInfo));
  }

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

    FocasResult result = read_machine_info(machine->ip, machine->port, info);

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
      } else {
        multi_info->failed_reads++;
        pool->failed_operations++;
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
