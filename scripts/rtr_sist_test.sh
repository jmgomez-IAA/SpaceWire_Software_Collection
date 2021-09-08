#!/bin/bash

# Los registros de SISt no son accesibles por RMAP!!!


./src/rd_rmap  -a 0x00000A08 -t 2

sleep 5
#################
# Port 1 Init
###################


sleep .5

# Port 1 control
echo "Port1 error cleared"
./src/rmap   -a 0x00000884 -d 0x000000FF -t 2
./src/rd_rmap  -a 0x00000884 -t 2

#################
# SIST port init
###################
echo "Leemos SIST port Status "
./src/rd_rmap  -a 0x000008CC -t 2
sleep .5
echo "Enable the SIST port en protection register"
./src/rmap -a 0x0000025C -d 0x55AA0001
sleep .5
echo " Read SIST port protection register"
./src/rd_rmap  -a 0x0000025C -t 2
sleep .5
echo "Enable  port control. Default: Disable (bit 10)"
./src/rmap -a 0x0000084C -d 0x00108020 -t 2
sleep .5
echo "Leemos control" 
./src/rd_rmap  -a 0x0000084C -t 2
sleep .5
echo "Leemos Status "
./src/rd_rmap  -a 0x000008CC -t 2


#################
# SIST pPOrt Packet init
###################
sleep .5
echo "Logical Address : 0xFE and Pysical path = 0x01"
#./src/rmap -a 0x00000200 -d 0xFE010000
sleep .5
# SIST port  Address0 - 7 Reg
./src/rd_rmap  -a 0x00000200 -t 2
sleep .5
echo " SIST port Seed Register 0xF"
./src/rmap -a 0x00000224  -d 0x00000713 -t 2
sleep .5
# SIST port Packet Length
 ./src/rmap -a 0x00000228  -d 0x0000000F -t 2
sleep .5
# Timer1 regisre shoud be cleared.
./src/rmap -a 0x00000244  -d 0x00000000 -t 2
 sleep .5
# SIST port  Control register: TX Start; AddresLen:1; RATE= 3. 
 ./src/rmap -a 0x0000022C  -d 0x81030000 -t 2

#################
# MK3 receive
###################

 #  Receive packets
./src/receiv -s 30170315 -t 1
#./src/receiv -s 30170315 -v -t 1

#################
# Debug
###################

#Transmitter Byte Count Register
./src/rd_rmap  -a 0x00000250 -t 2
#Transmitter Packet Count Register
./src/rd_rmap  -a 0x0000023C -t 2

# SIST port state register 
./src/rd_rmap  -a 0x0000024C -t 2

 
# Read Error registers 
./src/rd_rmap  -a 0x00000230 -t 2
./src/rd_rmap  -a 0x00000234 -t 2
./src/rd_rmap  -a 0x00000238 -t 2


# Port 1 control
./src/rd_rmap  -a 0x00000884 -t 2
	# Clear errors
./src/rmap  -a 0x00000884 -d 0x000000FF -t 2
 ./src/rd_rmap  -a 0x00000884 -t 2
 
 # SIST port status regiser. bit 3: Tx Almost Full ; bit 2: TX Full
./src/rd_rmap  -a 0x00000248 -t 2

