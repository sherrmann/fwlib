# FANUC FOCAS Development Roadmap

## 🎯 **Current Status**
✅ **Basic connectivity implemented** - Connect/disconnect to CNC  
✅ **CNC ID reading** - Get machine identifier  
✅ **Windows cross-compilation** - Complete build system  
✅ **Error handling** - Basic error reporting  

## 🚀 **Next Development Steps**

### **Phase 1: Machine Status & Program Info (Priority)**

#### **1.1 Read Current Program Name**
**Goal**: Display the currently loaded/running NC program

**Implementation**:
```c
// Add to util.c
int read_current_program(Config *conf, char *program_name) {
    unsigned short libh;
    short program_number;
    
    // Connect (reuse existing connection logic)
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return -1;
    }
    
    // Read current program number
    if (cnc_rdprgnum(libh, &program_number) != EW_OK) {
        cnc_freelibhndl(libh);
        return -1;
    }
    
    // Format program name (e.g., "O1234")
    snprintf(program_name, 16, "O%04d", program_number);
    
    cnc_freelibhndl(libh);
    return 0;
}
```

**Expected Output**:
```cmd
fanuc_example.exe --ip=10.100.74.17 --port=8193
connecting to machine at 10.100.74.17:8193...
machine id: 12345678-9abcdef0-12345678-9abcdef0
current program: O1234
```

#### **1.2 Read Machine Running Status**
**Goal**: Check if the machine is currently running/stopped

**Implementation**:
```c
// Add to util.c
int read_machine_status(Config *conf, char *status_info) {
    unsigned short libh;
    ODBST status;
    
    if (cnc_allclibhndl3(conf->ip, conf->port, 10, &libh) != EW_OK) {
        return -1;
    }
    
    // Read machine status
    if (cnc_statinfo(libh, &status) != EW_OK) {
        cnc_freelibhndl(libh);
        return -1;
    }
    
    // Interpret status
    switch(status.run) {
        case 0: strcpy(status_info, "STOPPED"); break;
        case 1: strcpy(status_info, "RUNNING"); break;
        case 2: strcpy(status_info, "PAUSED"); break;
        case 3: strcpy(status_info, "ALARM"); break;
        default: strcpy(status_info, "UNKNOWN"); break;
    }
    
    cnc_freelibhndl(libh);
    return 0;
}
```

**Expected Output**:
```cmd
fanuc_example.exe --ip=10.100.74.17 --port=8193
connecting to machine at 10.100.74.17:8193...
machine id: 12345678-9abcdef0-12345678-9abcdef0
current program: O1234
machine status: RUNNING
```

#### **1.3 Combined Status Display**
**Goal**: Single command shows all key information

**New main.c structure**:
```c
int main(int argc, char *argv[]) {
    char cncID[36];
    char program_name[16];
    char machine_status[16];
    Config conf;

    if (read_config(argc, argv, &conf)) {
        // ... existing error handling
    }

    // Get machine ID
    if (retrieve_id(&conf, cncID)) {
        return EXIT_FAILURE;
    }

    // Get current program
    if (read_current_program(&conf, program_name) != 0) {
        strcpy(program_name, "UNKNOWN");
    }

    // Get machine status
    if (read_machine_status(&conf, machine_status) != 0) {
        strcpy(machine_status, "UNKNOWN");
    }

    // Display all information
    printf("machine id: %s\n", cncID);
    printf("current program: %s\n", program_name);
    printf("machine status: %s\n", machine_status);

    return EXIT_SUCCESS;
}
```

### **Phase 2: Enhanced Program Information**

#### **2.1 Program Details**
- Current line number being executed
- Program size/length
- Execution time/remaining time

#### **2.2 Program Management**
- List all programs in memory
- Upload/download programs
- Program validation

### **Phase 3: Real-time Monitoring**

#### **3.1 Position Data**
- Current tool position (X, Y, Z coordinates)
- Work coordinate system
- Machine coordinate system

#### **3.2 Operational Data**
- Spindle speed (actual vs commanded)
- Feed rate (actual vs commanded)
- Tool number in use

### **Phase 4: Advanced Features**

#### **4.1 Alarm Management**
- Read active alarms
- Alarm history
- Alarm descriptions

#### **4.2 Production Monitoring**
- Part counters
- Cycle time
- Machine utilization

## 🛠 **Implementation Plan**

### **Step 1: Research FOCAS Functions**
Check `fwlib32.h` for these key functions:
```c
cnc_rdprgnum()    // Read program number
cnc_statinfo()    // Read machine status  
cnc_rdexecprog()  // Read executing program info
cnc_rdactpt()     // Read active part program
```

### **Step 2: Update Data Structures**
Add new structures to handle status data:
```c
typedef struct {
    char machine_id[36];
    char program_name[16];
    char status[16];
    int program_line;
} MachineInfo;
```

### **Step 3: Extend Configuration**
Add command-line options:
```bash
fanuc_example.exe --ip=IP --port=PORT --info=all
fanuc_example.exe --ip=IP --port=PORT --info=program
fanuc_example.exe --ip=IP --port=PORT --info=status
```

### **Step 4: Error Handling Enhancement**
Improve error reporting for specific functions:
```c
typedef enum {
    FOCAS_OK = 0,
    FOCAS_CONNECTION_FAILED = -1,
    FOCAS_PROGRAM_READ_FAILED = -2,
    FOCAS_STATUS_READ_FAILED = -3
} FocasResult;
```

## 📋 **File Changes Required**

### **New Files**:
- `src/machine_info.c` - Program and status reading functions
- `src/machine_info.h` - Data structures and function declarations

### **Modified Files**:
- `src/main.c` - Add program/status display
- `src/util.h` - Add new function declarations  
- `src/CMakeLists.txt` - Include new source files

### **Build System**:
- Update Makefile targets
- Add new compilation units
- Test new functionality

## 🧪 **Testing Strategy**

### **Unit Tests**:
- Test each FOCAS function individually
- Mock CNC responses for offline testing
- Error condition testing

### **Integration Tests**:
- Test with actual CNC machines
- Different FANUC models/series
- Network error scenarios

### **User Acceptance**:
- Verify output format is useful
- Test command-line interface
- Performance testing

## 📚 **Documentation Updates**

### **User Documentation**:
- Update README.md with new features
- Add usage examples
- Troubleshooting for new functions

### **Developer Documentation**:
- Code comments for new functions
- API documentation
- Build instructions for new features

## 🎯 **Success Criteria**

### **Phase 1 Complete When**:
✅ Can read current program name  
✅ Can read machine running status  
✅ Output is clear and useful  
✅ Error handling is robust  
✅ Works on Windows and Linux  

### **Immediate Next Steps**:
1. **Fix Error -16** - Get basic connectivity working
2. **Research FOCAS functions** - Verify `cnc_rdprgnum()` and `cnc_statinfo()` availability  
3. **Implement program reading** - Add program name functionality
4. **Implement status reading** - Add machine status functionality
5. **Test and validate** - Ensure robust operation

## 🔍 **FOCAS Functions to Investigate**

Priority functions for your use case:
```c
cnc_rdprgnum()     // Read program number - HIGH PRIORITY
cnc_statinfo()     // Read status info - HIGH PRIORITY  
cnc_rdexecprog()   // Read executing program info
cnc_rdactpt()      // Read active part program
cnc_rdprogline()   // Read current program line
cnc_rdprogname()   // Read program name (if available)
```

This roadmap focuses on your specific needs while providing a clear path for future expansion!
