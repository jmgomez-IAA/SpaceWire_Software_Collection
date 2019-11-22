#!/bin/bash 

./src/rd_rmap -a 0x00000A08 -t 1

./src/rd_rmap -a 0x00000A00 -t 1

# Read configuration write enable, in order to configuration problems
./src/rd_rmap -a 0x00000A10 -t 1

#Read link running status
./src/rd_rmap -a 0x00000A40 -t 1

