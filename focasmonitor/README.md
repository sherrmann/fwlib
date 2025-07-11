# FOCAS Monitor - Production Multi-Machine FANUC Monitoring

## Overview
`focasmonitor` is a production-ready Windows application for monitoring multiple FANUC CNC machines using the FOCAS library. It consolidates and improves upon the multi-machine capabilities developed in the examples folder.

**Comprehensive FANUC Support**: Includes all 14 FOCAS library DLLs supporting Series 0i, 15, 16, 30i, Power Mate, Data Server, NC Guide, and Ethernet connectivity across all FANUC machine generations.

## Features

### 🏭 **Multi-Machine Monitoring**
- Monitor up to 25 FANUC CNC machines simultaneously
- Connection pool management with automatic retry and reconnection
- Batch configuration via machine list files
- Real-time status monitoring across all machines

### 📊 **Comprehensive Machine Data**  
- **Machine Status**: Running/Stopped/Paused/Alarm states
- **Program Information**: Current program name, sequence number, execution details
- **Position Data**: Real-time X/Y/Z tool positions 
- **Speed Monitoring**: Feed rate and spindle speed
- **Alarm Management**: Active alarm detection and reporting

### 🔧 **Flexible Display Options**
- **Selective Information**: Display only specific data types (basic, program, position, speed, alarm)
- **Monitoring Modes**: One-shot status check or continuous monitoring
- **Output Formats**: Human-readable console output with optional verbose logging

### ⚙️ **Configuration Management**
- **Machine Lists**: Text-based configuration files for machine fleets
- **Command Line**: Dynamic machine addition and configuration
- **Environment Variables**: Integration with automation systems

## Quick Start

### Monitor Multiple Machines
```cmd
focasmonitor.exe --machines=factory_floor.txt --info=all
```

### Continuous Monitoring
```cmd
focasmonitor.exe --machines=factory_floor.txt --monitor --interval=30
```

### Add Machines Dynamically
```cmd
focasmonitor.exe --add=Mill1,192.168.1.100,8193 --add=Lathe1,192.168.1.101,8193 --info=basic
```

## Machine Configuration File Format

Create a text file with machine definitions:
```
# Production Floor Machines
# Format: name,ip,port
Mill-01,192.168.1.100,8193
Mill-02,192.168.1.101,8193
Lathe-01,192.168.1.102,8193
Lathe-02,192.168.1.103,8193
Grinder-01,192.168.1.104,8193
```

## Command Line Reference

### Options
```
--machines=<file>           Load machine list from configuration file
--add=<name,ip,port>        Add a machine (can be used multiple times)
--info=<type>               Information to display:
                            all      - All available information (default)
                            basic    - Machine ID and status only
                            program  - Program information
                            position - Tool position data
                            speed    - Speed and feed rate data
                            alarm    - Alarm status
--monitor                   Continuous monitoring mode
--interval=<seconds>        Monitoring interval (default: 30 seconds)
--output=<format>           Output format: console, json, csv
--verbose                   Enable verbose logging
--status                    Show connection pool status
--timeout=<seconds>         Connection timeout (default: 10 seconds)
--help                      Show help message
--version                   Show version information
```

### Examples
```cmd
# Monitor all machines from config file with basic info
focasmonitor.exe --machines=machines.txt --info=basic

# Add machines manually and monitor continuously
focasmonitor.exe --add=CNC1,192.168.1.10,8193 --add=CNC2,192.168.1.11,8193 --monitor --interval=60

# Check alarm status across all machines
focasmonitor.exe --machines=machines.txt --info=alarm

# Get position data with verbose output
focasmonitor.exe --machines=machines.txt --info=position --verbose

# Export data in JSON format
focasmonitor.exe --machines=machines.txt --output=json > status.json
```

## Architecture

### Core Components
- **Connection Pool**: Manages multiple FANUC machine connections with retry logic
- **Machine Info Reader**: Comprehensive data retrieval from FOCAS library
- **Configuration Manager**: Handles machine lists and command-line arguments
- **Display Engine**: Formats and outputs machine data in various formats
- **Monitor Loop**: Continuous monitoring with configurable intervals

### Data Flow
1. **Configuration**: Load machines from files or command line
2. **Connection**: Establish connections to all machines via connection pool
3. **Data Collection**: Read machine status, programs, positions, speeds, alarms
4. **Processing**: Format and filter data based on user selection
5. **Output**: Display results in specified format (console/JSON/CSV)
6. **Monitoring**: Repeat cycle if in continuous monitoring mode

## Windows Deployment

### Requirements
- Windows 7+ (32-bit or 64-bit)
- FANUC FOCAS library (Fwlib32.dll and related DLLs)
- Network access to FANUC CNC machines

### Package Contents
```
focasmonitor/
├── focasmonitor.exe         # Main executable (32-bit Windows)
├── Fwlib32.dll             # FANUC FOCAS library
├── fwlib*.dll              # Additional FANUC libraries
├── liblibconfig.dll        # Configuration library
├── machines.txt            # Sample machine configuration
├── README.md               # This documentation
└── LICENSE                 # License information
```

### Installation
1. Download and extract the focasmonitor package
2. Place machine configuration files in the same directory
3. Ensure network connectivity to FANUC machines
4. Run from command line or create shortcuts

## Integration

### Automation Systems
```cmd
# Scheduled monitoring with output redirection
focasmonitor.exe --machines=machines.txt --info=basic > daily_status.log

# JSON output for data processing
focasmonitor.exe --machines=machines.txt --output=json | process_data.py

# Alarm monitoring for alerts
focasmonitor.exe --machines=machines.txt --info=alarm --output=json | alert_system.exe
```

### Batch Scripts
```cmd
@echo off
echo Starting FANUC Machine Monitoring...
focasmonitor.exe --machines=production_line.txt --monitor --interval=120
if %errorlevel% neq 0 (
    echo Error: Monitoring failed with code %errorlevel%
    pause
)
```

## Development

### Building from Source
```bash
cd /workspaces/fwlib/focasmonitor
make build-windows
```

### Testing
```bash
make test-windows
```

### Contributing
This project consolidates the multi-machine capabilities developed in `/workspaces/fwlib/examples/c/` into a production-ready application. See the examples folder for development history and detailed implementation notes.

## Troubleshooting

### Common Issues
1. **Connection Failures**: Verify machine IP addresses and network connectivity
2. **Missing DLLs**: Ensure all FANUC DLLs are in the same directory as focasmonitor.exe
3. **Configuration Errors**: Check machine list file format and syntax
4. **Permission Issues**: Run as administrator if needed for network access

### Debug Mode
```cmd
focasmonitor.exe --machines=machines.txt --verbose --info=all
```

## License
This software uses the FANUC FOCAS library and is subject to FANUC licensing terms.
