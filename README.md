# UDP Server - AM243x-lp FreeRTOS UDP Client

A real-time embedded UDP client application running on Texas Instruments AM243x-lp processor with FreeRTOS, designed for efficient sensor data transmission and network communication.

## Overview

This project implements a multi-threaded UDP client for the AM243x-lp development board. It features a data generator thread that periodically collects sensor telemetry (temperature, voltage) into a circular buffer, and a dedicated UDP task that handles network transmission and reception. The application demonstrates best practices for embedded network programming using FreeRTOS synchronization primitives and the lwIP TCP/IP stack.

## Project Structure

```
enet_cpsw_udpclient_am243x-lp_r5fss0-0_freertos_ti-arm-clang/
├── app_main.c                    # Main application initialization and network setup
├── app_main.h                    # Main header definitions
├── app_udpclient.c               # UDP client implementation with data generator and UDP tasks
├── app_udpclient.h               # UDP client interface
├── app_cpswconfighandler.c       # CPSW network configuration handler
├── app_cpswconfighandler.h       # CPSW handler interface
├── main.c                        # Entry point and FreeRTOS task setup
├── makefile_ccs_bootimage_gen    # Boot image generation makefile
├── syscfg_c.rov.xs               # System configuration ROV script
├── Debug/                        # Build artifacts and generated files
│   └── syscfg/                   # Auto-generated driver and board configuration files
└── targetConfigs/                # Target configuration files for AM2434_ALX

```

## Key Features

- **Multi-threaded Architecture**: Data generator thread (100ms interval) and UDP server task running concurrently
- **Circular Buffer Management**: Efficient sensor data buffering with mutex protection (max 30 entries)
- **Thread Synchronization**: Uses FreeRTOS semaphores and task notifications for thread-safe data access
- **UDP Network Communication**: Bidirectional UDP communication on port 8016
- **Sensor Data Telemetry**: Captures timestamp, log type, temperature, and voltage readings
- **DHCP Support**: Automatic IP address allocation via DHCP
- **Network Status Monitoring**: Link change detection and network interface status callbacks

## Languages Used

- **C** - Primary language for the entire embedded application

## Technologies Used

- **FreeRTOS** - Real-time operating system providing task scheduling and synchronization
- **lwIP** - Lightweight TCP/IP network stack for embedded systems
- **CPSW (Common Platform Switch)** - Ethernet MAC controller interface
- **Enet Driver** - TI Enet driver layer for network hardware abstraction
- **DHCP** - Dynamic Host Configuration Protocol for automatic IP assignment

## Libraries and Frameworks Used

### Core Frameworks
- **FreeRTOS** - RTOS for real-time task management, semaphores, and inter-task communication
  - `task.h` - Task management APIs
  - `semphr.h` - Binary and counting semaphore implementations

### Network Stack
- **lwIP (Lightweight IP)** - Minimal TCP/IP implementation
  - `lwip/opt.h` - lwIP configuration options
  - `lwip/sys.h` - System abstraction layer
  - `lwip/tcpip.h` - TCP/IP core stack
  - `lwip/dhcp.h` - DHCP client implementation
  - `lwip/sockets.h` - BSD-like socket interface

### TI Drivers and Board Support
- **TI Board Configuration** - Hardware abstraction and board-specific setup
  - `ti_board_config.h/.c` - Board initialization
  - `ti_board_open_close.h/.c` - Board resource management
  
- **TI Drivers Configuration** - Device driver initialization
  - `ti_drivers_config.h/.c` - Driver setup and configuration
  - `ti_drivers_open_close.h/.c` - Driver lifecycle management

- **TI Enet Configuration** - Network driver configuration
  - `ti_enet_config.h/.c` - Ethernet controller setup
  - `ti_enet_open_close.h/.c` - Ethernet driver lifecycle
  - `ti_enet_lwipif.h/.c` - lwIP interface adapter for Enet driver

- **TI DPL (Driver Porting Layer)** - Low-level hardware abstraction
  - `kernel/dpl/TaskP.h` - Task priority definitions
  - `kernel/dpl/ClockP.h` - Clock and timing utilities
  - `kernel/dpl/CacheP.h` - Cache management
  - `kernel/dpl/QueueP.h` - Queue primitives
  - `kernel/dpl/DebugP.h` - Debug utilities

### Application Utilities
- **Enet Application Utilities** - Helper functions for Enet driver
  - `enet_apputils.h` - Print and assertion utilities
  - `enet_board.h` - Board-specific network setup

## Hardware Target

- **Processor**: TI AM243x-lp (ARM Cortex-R5 FSS0-0 core)
- **Compiler**: TI ARM Clang (`ti-arm-clang`)
- **IDE**: Code Composer Studio (CCS)
- **Toolchain**: TI Code Generation Tools

## Main Components

### 1. Data Generator Task (`AppSocket_dataGeneratorTask`)
- Runs every 100ms
- Generates synthetic sensor data (temperature and voltage)
- Writes data to circular buffer with mutex protection
- Notifies UDP task when new data is available

### 2. UDP Server Task (`AppSocket_udpServerTask`)
- Creates UDP socket on port 8016
- Receives incoming UDP messages
- Sends buffered sensor data to connected clients
- Thread-safe buffer access using semaphore protection

### 3. Network Initialization (`App_setupNetworkStack`)
- Sets up network interface with DHCP
- Registers callbacks for link status changes
- Configures multicast entries using CPSW

### 4. CPSW Configuration Handler (`EnetApp_addMCastEntry`)
- Manages multicast address entries
- Handles MDIO link status changes
- Port link status monitoring

## Build System

- **Build Tool**: Make (via CCS)
- **Debug Configuration**: Generated in `Debug/` directory
- **Optimization**: Configured for ARM Cortex-R5 architecture
- **Generated Files**: Automatically created by SysConfig tool in `Debug/syscfg/`

## Network Configuration

- **Protocol**: UDP/IP
- **Port**: 8016
- **DHCP**: Enabled for dynamic IP assignment
- **Data Payload**: 256-byte transmission buffer, 128-byte receive buffer
- **Circular Buffer**: 30-entry log buffer for sensor data

## Dependencies

- TI SDK for AM243x
- Code Composer Studio IDE
- TI ARM Clang compiler toolchain
- FreeRTOS kernel
- lwIP TCP/IP stack

## Getting Started

1. Import project into Code Composer Studio
2. Configure system resources via SysConfig tool
3. Build the project (generates syscfg files in `Debug/syscfg/`)
4. Flash the binary to AM243x-lp board
5. UDP client will initialize network on startup and begin sending sensor data

## Data Structure

Sensor log entries contain:
- `timestamp` (uint32_t) - millisecond timestamp
- `type` (uint8_t) - Log type (0=INFO, 1=WARNING, 2=ERROR)
- `temperature` (float) - Temperature reading in Celsius
- `voltage` (float) - Voltage reading in Volts

## Synchronization Mechanisms

- **Mutex Semaphore** - Protects circular buffer access between data generator and UDP tasks
- **Task Notifications** - Lightweight event signaling from data generator to UDP task
- **FreeRTOS Tasks** - Cooperative multi-tasking with configurable priorities

## License

Components are licensed under various terms including BSD 2-Clause and BSD 3-Clause licenses. See individual source files for copyright and license information.

---

**Project Name**: ENET CPSW UDP Client  
**Target Board**: AM243x-lp  
**RTOS**: FreeRTOS  
**Network Stack**: lwIP
