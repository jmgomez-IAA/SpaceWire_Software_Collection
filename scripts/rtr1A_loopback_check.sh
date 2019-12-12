#!/bin/bash

source router_defs.sh
#.router_defs.sh

TIMEOUT=1

#Check communication on interface 
  
 ./src/loopback -v -t 1 -a 0x2F
 ./src/loopback -v -t 1 -a 0x2E
 ./src/loopback -v -t 1 -a 0x2F
 ./src/loopback -v -t 1 -a 0x2F
 ./src/loopback -v -t 1 -a 0x30
 ./src/loopback -v -t 1 -a 0x31
 ./src/loopback -v -t 1 -a 0x32
 ./src/loopback -v -t 1 -a 0x32
 ./src/loopback -v -t 1 -a 0x33
 ./src/loopback -v -t 1 -a 0x33
 ./src/loopback -v -t 1 -a 0x48
 ./src/loopback -v -t 1 -a 0x49
 ./src/loopback -v -t 1 -a 0x4F

#DPU 1: Address 0x2E on port RTR_PORT_3
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_46}

#DPU 2: Address 0x2F on port RTR_PORT_2
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_47}

#DPU 3: Address 0x30 on port RTR_PORT_5
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_48}

#DPU 4: Address 0x31 on port RTR_PORT16
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_49}

#DPU 5: Address 0x32 on port RTR_PORT12
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_50}

#DPU 6: Address 0x33 on port RTR_PORT9
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_51}

#DPU 6: Address 0x33 
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_51}

#PSU Cntrl A: Address 0x48 on port RTR_PORT6
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_72}

#PSU Cntrl B: Address 0x49 on port RTR_PORT14
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_73}

#Test port: Address 0x4F on port RTR_PORT1
./src/loopback -v -t ${TIMEOUT} -a ${RTR_RTPMAP_ADD_79}