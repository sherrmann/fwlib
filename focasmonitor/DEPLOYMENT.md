# FOCAS Monitor - Deployment Guide

## 🚀 Quick Deployment

### **System Requirements**
- Windows 7 or later (32-bit or 64-bit)
- Network connectivity to FANUC CNC machines
- FANUC machines with FOCAS support enabled

### **Installation**
1. Copy the entire `release/` folder to your Windows machine
2. Place it in a convenient location (e.g., `C:\FocasMonitor\`)
3. Ensure all DLL files remain in the same directory as `focasmonitor.exe`

### **Package Contents**
The release package includes all necessary files:

**Core Application:**
- `focasmonitor.exe` - Main executable

**FANUC FOCAS Libraries (14 DLLs):**
- `Fwlib32.dll` - Main FOCAS library
- `Fwlib0i.dll`, `Fwlib0iB.dll` - Series 0i support
- `Fwlib150.dll`, `Fwlib15i.dll` - Series 15 support  
- `Fwlib160.dll`, `Fwlib16W.dll` - Series 16 support
- `Fwlibpm.dll`, `Fwlibpmi.dll` - Power Mate support
- `fwlib0DN.dll`, `fwlib0iD.dll` - Data Server support
- `fwlib30i.dll` - Series 30i support
- `fwlibNCG.dll` - NC Guide support
- `fwlibe1.dll` - Ethernet support

**Configuration & Documentation:**
- `machines.txt` - Sample machine configuration
- `README.md` - Usage documentation

⚠️ **Important**: All DLL files must remain in the same directory as the executable for proper operation.

### **Basic Setup**
1. **Configure Machines**: Edit `machines.txt` with your machine details
   ```
   Mill-01,192.168.1.100,8193
   Lathe-01,192.168.1.101,8193
   ```

2. **Test Connection**: Run a basic status check
   ```cmd
   focasmonitor.exe --machines=machines.txt --info=basic
   ```

3. **Start Monitoring**: Begin continuous monitoring
   ```cmd
   focasmonitor.exe --machines=machines.txt --monitor --interval=60
   ```

## 📋 **Common Usage Patterns**

### **Production Floor Monitoring**
```cmd
# Basic status overview
focasmonitor.exe --machines=production_floor.txt --info=basic

# Detailed monitoring with 30-second updates
focasmonitor.exe --machines=production_floor.txt --monitor --interval=30
```

### **Quality Control**
```cmd
# Position monitoring for precision checking
focasmonitor.exe --machines=qc_machines.txt --info=position

# Alarm monitoring for issues
focasmonitor.exe --machines=all_machines.txt --info=alarm
```

### **Data Collection**
```cmd
# Export to JSON for analysis
focasmonitor.exe --machines=machines.txt --output=json > daily_report.json

# Export to CSV for Excel
focasmonitor.exe --machines=machines.txt --output=csv > status_log.csv
```

### **Troubleshooting**
```cmd
# Verbose output for debugging
focasmonitor.exe --machines=machines.txt --verbose --info=all

# Connection pool status
focasmonitor.exe --machines=machines.txt --status
```

## 🔧 **Configuration Files**

### **Machine List Format**
```
# Lines starting with # are comments
# Format: name,ip_address,port

# Production machines
Mill-01,192.168.1.100,8193
Mill-02,192.168.1.101,8193
Lathe-01,192.168.1.102,8193

# Quality control
CMM-01,192.168.1.120,8193
```

### **Network Configuration**
- Ensure FANUC machines have FOCAS enabled
- Verify network connectivity (ping test)
- Check firewall settings for port 8193 (or custom port)
- Confirm machine IP addresses are correct

## 🛠️ **Automation & Integration**

### **Scheduled Monitoring**
Create a batch file for scheduled execution:
```cmd
@echo off
cd /d "C:\FocasMonitor"
focasmonitor.exe --machines=machines.txt --output=csv >> logs\daily_%date%.csv
if %errorlevel% neq 0 (
    echo Error occurred at %time% >> logs\error.log
)
```

### **Alert System**
Monitor for alarms and trigger notifications:
```cmd
@echo off
focasmonitor.exe --machines=machines.txt --info=alarm --output=json > temp_alarms.json
findstr "true" temp_alarms.json >nul
if %errorlevel% equ 0 (
    echo ALERT: Machine alarm detected at %time%
    rem Add your notification command here
)
```

### **Data Processing**
Export data for further processing:
```cmd
# Daily data collection
focasmonitor.exe --machines=machines.txt --output=json > data\%date%_machines.json

# Performance monitoring
focasmonitor.exe --machines=machines.txt --info=speed --output=csv >> performance_log.csv
```

## 🔍 **Troubleshooting**

### **Common Issues**

1. **"Failed to connect to cnc!" Errors**
   - Check network connectivity: `ping <machine_ip>`
   - Verify FOCAS is enabled on the machine
   - Confirm correct IP address and port
   - Check firewall settings

2. **"DLL not exist error" Messages**
   - Ensure all FANUC DLLs are in the same directory
   - Check that Fwlib32.dll is present
   - Verify file permissions

3. **"No machines specified" Error**
   - Check machines.txt file format
   - Verify file path is correct
   - Ensure file is not empty or all commented

4. **Permission Issues**
   - Run Command Prompt as Administrator
   - Check file/folder permissions
   - Verify network access rights

### **Debug Mode**
```cmd
# Full verbose output
focasmonitor.exe --machines=machines.txt --verbose --info=all

# Check what's happening with connections
focasmonitor.exe --machines=machines.txt --status --verbose
```

## 📊 **Performance Guidelines**

### **Recommended Settings**
- **Monitor Interval**: 30-60 seconds for most applications
- **Machine Count**: Up to 50 machines per instance
- **Info Type**: Use `basic` for frequent monitoring, `all` for detailed analysis

### **Scaling Considerations**
- For more than 50 machines, run multiple instances with different machine lists
- Adjust monitor interval based on network performance
- Use selective info types to reduce data volume

## 📝 **Best Practices**

1. **Machine Lists**: Organize by production area or machine type
2. **Monitoring**: Start with longer intervals and adjust as needed
3. **Data Export**: Regular exports for historical analysis
4. **Error Handling**: Monitor error logs and connection failures
5. **Updates**: Keep machine IP addresses current

## 📞 **Support Information**

### **Log Files**
- Monitor output can be redirected to files for analysis
- Use `--verbose` flag for detailed logging
- Check Windows Event Viewer for system-level issues

### **Performance Monitoring**
- Monitor CPU and memory usage during operation
- Network utilization should be minimal
- Response times depend on network latency to machines

---

**FOCAS Monitor is designed for reliable, long-term operation in industrial environments. Follow these guidelines for optimal performance and reliability.**
