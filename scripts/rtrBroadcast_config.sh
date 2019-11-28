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

#										BroadCast (0xFE)
#								|							|	
#								|--- GRSPW4 --- GRSPW7   ---|
##  DUNDEE#1 -->  GRSPW#1  -----|--- GRSPW5 --- GRSPW8 	 ---|---GRSPW#2 ---> DUNDEE#2
#								|--- GRPSW6 --- GRSPW9   ---|
#								|							|	


#Enable the interface
./src/rmap -a ${RTR_PCTRL_PORT_1} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_2} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_3} -d  ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}

./src/rmap -a ${RTR_PCTRL_PORT_4} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_5} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_6} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_7} -d  ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
./src/rmap -a ${RTR_PCTRL_PORT_8} -d  ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}

#./src/rmap -a ${RTR_PCTRL_PORT_9} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_10} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_11} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_12} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_13} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_14} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_15} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_16} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_17} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}
#./src/rmap -a ${RTR_PCTRL_PORT_18} -d ${RTR_PCTRL_PORT_VALUE_START_AND_100MHZ} -t ${TIMEOUT}

#
#Set routing table - Port mapping
#Test port: Address 0x4F
./src/rmap -a ${RTR_RTPMAP_ADD_79}  -d ${RTR_PORT_1}  -t ${TIMEOUT}
#./src/rmap -a ${RTR_RTPMAP_ADD_7}   -d ${RTR_PORT_2}  -t ${TIMEOUT}
#./src/rmap -a ${RTR_RTPMAP_ADD_8}   -d ${RTR_PORT_3}  -t ${TIMEOUT}

# Broadcast interfaces.
if_to_broadcast=$((RTR_PORT_4 | RTR_PORT_5 | RTR_EN_PD))
# //ICU Main High priority:_Address 0xFE
echo "Configure the interfaces ${if_to_broadcast} for address 254."
./src/rmap -a ${RTR_RTPMAP_ADD_254}  -d  ${if_to_broadcast}  -t ${TIMEOUT}

#Routing table - Address control (Enable address)
./src/rmap -a ${RTR_RTACTRL_ADD_79}  -d ${RTR_RTACTRL_ADD_VALUE_HD_NO}  -t ${TIMEOUT}

#Active Header deletion on Addr 254 so it is forwarded to the next hop 0x2
./src/rmap -a ${RTR_RTACTRL_ADD_254} -d ${RTR_RTACTRL_ADD_VALUE_HD_YES}  -t ${TIMEOUT}


#Transmit the message in broadcast to 0xFE and receive in 0x2.


