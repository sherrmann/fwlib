#include "focasmonitor.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

const char *focas_result_to_string(FocasResult result) {
  switch (result) {
  case FOCAS_OK:
    return "Success";
  case FOCAS_CONNECTION_FAILED:
    return "Connection failed";
  case FOCAS_PROGRAM_READ_FAILED:
    return "Program read failed";
  case FOCAS_STATUS_READ_FAILED:
    return "Status read failed";
  case FOCAS_ID_READ_FAILED:
    return "ID read failed";
  case FOCAS_SEQUENCE_READ_FAILED:
    return "Sequence read failed";
  case FOCAS_POSITION_READ_FAILED:
    return "Position read failed";
  case FOCAS_SPEED_READ_FAILED:
    return "Speed read failed";
  case FOCAS_ALARM_READ_FAILED:
    return "Alarm read failed";
  case FOCAS_POOL_FULL:
    return "Connection pool full";
  case FOCAS_MACHINE_NOT_FOUND:
    return "Machine not found";
  case FOCAS_INVALID_CONFIG:
    return "Invalid configuration";
  default:
    return "Unknown error";
  }
}

const char *connection_state_to_string(ConnectionState state) {
  switch (state) {
  case CONN_DISCONNECTED:
    return "Disconnected";
  case CONN_CONNECTING:
    return "Connecting";
  case CONN_CONNECTED:
    return "Connected";
  case CONN_ERROR:
    return "Error";
  case CONN_BUSY:
    return "Busy";
  default:
    return "Unknown";
  }
}

OutputFormat parse_output_format(const char *format_str) {
  if (strcmp(format_str, "json") == 0) {
    return OUTPUT_JSON;
  } else if (strcmp(format_str, "csv") == 0) {
    return OUTPUT_CSV;
  } else {
    return OUTPUT_CONSOLE;
  }
}

void print_machine_info(const MachineInfo *info, const char *machine_name) {
  printf("=== %s ===\n", machine_name);
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
  printf("\n--- Speed Information ---\n");
  printf("Feed Rate: %d mm/min\n", info->speed.feed_rate);
  printf("Spindle Speed: %d RPM\n", info->speed.spindle_speed);

  // Alarm information
  printf("\n--- Alarm Information ---\n");
  printf("Alarm Status: %s\n", info->alarm.has_alarm ? "ACTIVE" : "NONE");
  if (info->alarm.has_alarm) {
    printf("Alarm Code: %d\n", info->alarm.alarm_status);
  }

  printf("Last Updated: %s", ctime(&info->last_updated));
  printf("\n");
}

void print_selective_machine_info(const MachineInfo *info,
                                  const char *machine_name,
                                  const char *info_type) {
  if (strcmp(info_type, "basic") == 0) {
    printf("%-15s | %-35s | %s\n", machine_name, info->machine_id,
           info->status);
  } else if (strcmp(info_type, "program") == 0) {
    printf("%-15s | %-10s | N%-8ld | %s\n", machine_name, info->program_name,
           info->sequence_number, info->status);
  } else if (strcmp(info_type, "position") == 0) {
    printf("%-15s | X:%8.3f | Y:%8.3f | Z:%8.3f | %s\n", machine_name,
           info->position.x_abs, info->position.y_abs, info->position.z_abs,
           info->status);
  } else if (strcmp(info_type, "speed") == 0) {
    printf("%-15s | Feed:%5d mm/min | Spindle:%5d RPM | %s\n", machine_name,
           info->speed.feed_rate, info->speed.spindle_speed, info->status);
  } else if (strcmp(info_type, "alarm") == 0) {
    printf("%-15s | Alarm: %-6s", machine_name,
           info->alarm.has_alarm ? "ACTIVE" : "NONE");
    if (info->alarm.has_alarm) {
      printf(" (Code: %d)", info->alarm.alarm_status);
    }
    printf(" | %s\n", info->status);
  } else {
    // "all" or unknown - show complete info
    print_machine_info(info, machine_name);
  }
}

void print_machine_info_json(const MachineInfo *info,
                             const char *machine_name) {
  printf("    {\n");
  printf("      \"name\": \"%s\",\n", machine_name);
  printf("      \"machine_id\": \"%s\",\n", info->machine_id);
  printf("      \"program_name\": \"%s\",\n", info->program_name);
  printf("      \"program_number\": %d,\n", info->program_number);
  printf("      \"status\": \"%s\",\n", info->status);
  printf("      \"sequence_number\": %ld,\n", info->sequence_number);
  printf("      \"position\": {\n");
  printf("        \"x_abs\": %.3f,\n", info->position.x_abs);
  printf("        \"y_abs\": %.3f,\n", info->position.y_abs);
  printf("        \"z_abs\": %.3f,\n", info->position.z_abs);
  printf("        \"x_rel\": %.3f,\n", info->position.x_rel);
  printf("        \"y_rel\": %.3f,\n", info->position.y_rel);
  printf("        \"z_rel\": %.3f\n", info->position.z_rel);
  printf("      },\n");
  printf("      \"speed\": {\n");
  printf("        \"feed_rate\": %d,\n", info->speed.feed_rate);
  printf("        \"spindle_speed\": %d\n", info->speed.spindle_speed);
  printf("      },\n");
  printf("      \"alarm\": {\n");
  printf("        \"has_alarm\": %s,\n",
         info->alarm.has_alarm ? "true" : "false");
  printf("        \"alarm_status\": %d\n", info->alarm.alarm_status);
  printf("      },\n");
  printf("      \"last_updated\": %ld\n", info->last_updated);
  printf("    }");
}

void print_machine_info_csv(const MachineInfo *info, const char *machine_name,
                            bool header) {
  if (header) {
    printf("machine_name,machine_id,program_name,program_number,status,"
           "sequence_number,");
    printf("x_abs,y_abs,z_abs,x_rel,y_rel,z_rel,feed_rate,spindle_speed,has_"
           "alarm,alarm_status,last_updated\n");
  } else {
    printf("%s,%s,%s,%d,%s,%ld,", machine_name, info->machine_id,
           info->program_name, info->program_number, info->status,
           info->sequence_number);
    printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,", info->position.x_abs,
           info->position.y_abs, info->position.z_abs, info->position.x_rel,
           info->position.y_rel, info->position.z_rel);
    printf("%d,%d,%s,%d,%ld\n", info->speed.feed_rate,
           info->speed.spindle_speed, info->alarm.has_alarm ? "true" : "false",
           info->alarm.alarm_status, info->last_updated);
  }
}

void print_multi_machine_info(const MultiMachineInfo *multi_info,
                              const char *info_type, OutputFormat format) {
  if (format == OUTPUT_JSON) {
    printf("{\n");
    printf("  \"collection_time\": %ld,\n", multi_info->collection_time);
    printf("  \"machine_count\": %d,\n", multi_info->machine_count);
    printf("  \"successful_reads\": %d,\n", multi_info->successful_reads);
    printf("  \"failed_reads\": %d,\n", multi_info->failed_reads);
    printf("  \"machines\": [\n");

    for (int i = 0; i < multi_info->machine_count; i++) {
      char machine_name[64];
      snprintf(machine_name, sizeof(machine_name), "Machine_%d", i + 1);
      print_machine_info_json(&multi_info->machines[i], machine_name);
      if (i < multi_info->machine_count - 1) {
        printf(",");
      }
      printf("\n");
    }

    printf("  ]\n");
    printf("}\n");

  } else if (format == OUTPUT_CSV) {
    print_machine_info_csv(NULL, NULL, true); // Print header

    for (int i = 0; i < multi_info->machine_count; i++) {
      char machine_name[64];
      snprintf(machine_name, sizeof(machine_name), "Machine_%d", i + 1);
      print_machine_info_csv(&multi_info->machines[i], machine_name, false);
    }

  } else {
    // Console format
    if (strcmp(info_type, "basic") == 0) {
      printf("%-15s | %-35s | %s\n", "Machine", "Machine ID", "Status");
      printf("%-15s-+-%-35s-+-%s\n", "---------------",
             "-----------------------------------", "----------");
    } else if (strcmp(info_type, "program") == 0) {
      printf("%-15s | %-10s | %-10s | %s\n", "Machine", "Program", "Sequence",
             "Status");
      printf("%-15s-+-%-10s-+-%-10s-+-%s\n", "---------------", "----------",
             "----------", "----------");
    } else if (strcmp(info_type, "position") == 0) {
      printf("%-15s | %-10s | %-10s | %-10s | %s\n", "Machine", "X (mm)",
             "Y (mm)", "Z (mm)", "Status");
      printf("%-15s-+-%-10s-+-%-10s-+-%-10s-+-%s\n", "---------------",
             "----------", "----------", "----------", "----------");
    } else if (strcmp(info_type, "speed") == 0) {
      printf("%-15s | %-15s | %-15s | %s\n", "Machine", "Feed (mm/min)",
             "Spindle (RPM)", "Status");
      printf("%-15s-+-%-15s-+-%-15s-+-%s\n", "---------------",
             "---------------", "---------------", "----------");
    } else if (strcmp(info_type, "alarm") == 0) {
      printf("%-15s | %-12s | %s\n", "Machine", "Alarm Status",
             "Machine Status");
      printf("%-15s-+-%-12s-+-%s\n", "---------------", "------------",
             "---------------");
    }

    for (int i = 0; i < multi_info->machine_count; i++) {
      char machine_name[64];
      snprintf(machine_name, sizeof(machine_name), "Machine_%d", i + 1);
      print_selective_machine_info(&multi_info->machines[i], machine_name,
                                   info_type);
    }

    printf("\nSummary: %d machines, %d successful reads, %d failed reads\n",
           multi_info->machine_count, multi_info->successful_reads,
           multi_info->failed_reads);
    printf("Collection time: %s", ctime(&multi_info->collection_time));
  }
}
