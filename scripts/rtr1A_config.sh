#!/bin/bash

source router_defs.sh
#.router_defs.sh

TIMEOUT=1
#ROUTER_LINK_RUN_STATUS_REGISTER=0x00000A40

./src/rd_rmap -a 0x00000A08 -t 1

./src/rd_rmap -a 0x00000A00 -t 1

# Read configuration write enable} in order to configuration problems
./src/rd_rmap -a 0x00000A10 -t 1

#Read link running status
./src/rd_rmap -a ${ROUTER_LINK_RUN_STATUS_REGISTER} -t ${TIMEOUT}

#Start configuring Addresss

#Enable the interface
./src/rmap -a ${RTR_PCTRL_PORT_1} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_2} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_3} -d  ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_4} -d ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_5} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_6} -d ${RTR_PCTRL_PORT_VALUE_START_AND_10MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_7} -d  ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_8} -d  ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_9} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_10} -d ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_11} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_12} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_13} -d ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_14} -d ${RTR_PCTRL_PORT_VALUE_START_AND_10MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_15} -d ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_16} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_17} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_18} -d ${RTR_PCTRL_PORT_VALUE_DISABLE_PORT} -t ${TIMEOUT}

#
#Set routing table - Port mapping
#
# //ICU Main High priority:_Address 0x20
./src/rmap -a ${RTR_RTPMAP_ADD_32}  -d  ${RTR_PORT_17}  -t ${TIMEOUT}
#ICU Main High priority: Address 0x21
./src/rmap -a ${RTR_RTPMAP_ADD_33}   -d ${RTR_PORT_17}  -t ${TIMEOUT}
#DPU 1: Address 0x2E
./src/rmap -a ${RTR_RTPMAP_ADD_46}   -d ${RTR_PORT_3}  -t ${TIMEOUT}
#DPU 2: Address 0x2F
./src/rmap -a ${RTR_RTPMAP_ADD_47}  -d ${RTR_PORT_2}  -t ${TIMEOUT}
#DPU 3: Address 0x30
./src/rmap -a ${RTR_RTPMAP_ADD_48}  -d ${RTR_PORT_5}  -t ${TIMEOUT}
#DPU 4: Address 0x31
./src/rmap -a ${RTR_RTPMAP_ADD_49}  -d ${RTR_PORT_16}  -t ${TIMEOUT}
#DPU 5: Address 0x32
./src/rmap -a ${RTR_RTPMAP_ADD_50}  -d ${RTR_PORT_12}  -t ${TIMEOUT}
#DPU 6: Address 0x33
./src/rmap -a ${RTR_RTPMAP_ADD_51}  -d ${RTR_PORT_9}  -t ${TIMEOUT}
#PSU Cntrl A: Address 0x48
./src/rmap -a ${RTR_RTPMAP_ADD_72}  -d ${RTR_PORT_6}  -t ${TIMEOUT}
#PSU Cntrl B: Address 0x49
./src/rmap -a ${RTR_RTPMAP_ADD_73}  -d ${RTR_PORT_14}  -t ${TIMEOUT}
#Test port: Address 0x4F
./src/rmap -a ${RTR_RTPMAP_ADD_79}  -d ${RTR_PORT_1}  -t ${TIMEOUT}

#Routing table - Address control (Enable address)
./src/rmap -a ${RTR_RTACTRL_ADD_32}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_33}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_46}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_47}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_48}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_49}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_50}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_51}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_72}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_73}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_79}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}
./src/rmap -a ${RTR_RTACTRL_ADD_254} -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}

