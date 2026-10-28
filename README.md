# S2-Lab: Ground Equipment Debugging Station

## 1. Project Purpose

Replacement for a ground equipment unit equipped with pressure and weight sensors.

Displays data from ADC channels, received over the network from the ESZ7-SIU unit (based on a Xilinx Zynq SoC).

## 2. S2-Lab

```
S2-Lab Ver. 2.0.1 May 29 2026 (built May 29 2026 11:28:22)

```

#### Operational Ports

Ports used for communication with the ESZ7-SIU:

* 16000 – UDP port for receiving data via the ATLAS protocol.
* 8200 – TCP/IP port for sending commands to the ESZ7-SIU.

## Project Build

1. The application is developed using Qt 5.

Project configuration is managed via CMake (see `CMakeLists.txt`).

The following paths must be added to the OS environment:
`C:\Qt-5.12.3\5.12.3\bin;C:\Qt-5.12.3-off\Tools\mingw730_64\bin`

2. Building

2.1 The project can be built using Qt Creator.

2.2 Building via the command line:
```
> mkdir build
> cmake -G "MinGW Makefiles" -S . -B build -DCMAKE_BUILD_TYPE=Release
> cmake --build ./build -j26 --config Release
```

```
$ cmake --build ./build --target clean
```
