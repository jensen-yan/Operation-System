# pktRxTx

## Introduction

### Brief Introduction

pktRxTx is able to listen network interface，and send/receive network packets.

The tool can be used on Windows，Linux，or Mac OS.

WinPcap is called by pktRxTx on Windows. And it is libpcap for Linux and Mac OS. In addition, WpdPack is needed when developed on Windows.


### File Description

./   
├── Makefile    
├── README.md   
├── build.bat   
├── common.h   
├── pktRxTx.c   
├── pktheader.h   
└── print.c   

1. pktheader.h

&ensp;&ensp;&ensp;&ensp;This header file declares Ethernet header, TCP header, UDP header, and IP header.

2. pktRxTx.c

&ensp;&ensp;&ensp;&ensp;File contains main function.

3. Makefile

&ensp;&ensp;&ensp;&ensp;Makefile is used for compiling on Linux/Mac OS.

4. build.bat

&ensp;&ensp;&ensp;&ensp;This patch script is used for building software on Windows.

## Runing Mode

### echo Mode

pktRxTx will send back all packets received after modifying src/dest MAC address when running on echo mode.

### listening mode

When running on listening mode, pktRxTx just keeps listening interface, and parsing commandline given by users.

For example, pktRxTx will send 5 packets when user  types in "send 5".


## Build and Running

###  Linux/Mac

```bash
$  cd pktRxTx/
$  make
$  sudo ./pktRxTx
```

Note. Root privelidges are needed when running on Linux or Mac OS.

### Windows

```bash
open command prompt.
> cd <path-to-project>
> build.bat  
> pktRxTx.exe
```

Note. You need to modify "-I" option according to self needs, Or you can add WpaPack/Include/ and WpdPcak/Lib/ to the environment variables for your machine.

## Note

1. Src/dest MAC address is fixed for now, so user needs to modify MAC address manually before compiling.

2. It is recommended that installing Visual Studio for Windows platform, or you may get error like "can not find xx.dll".
