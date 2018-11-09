
#ifndef __MEU_RTR_CONFIG__
#define  __MEU_RTR_CONFIG__



/////////////////////////////////////////////////
//  Define the Logical address of the devices
/////////////////////////////////////////////////
#define MEU_NDPU_DEFAULT_LA 0xFE
#define MEU_NDPU_TEST_LA 0x29A

#define MEU1_ICUA_LA 0x1C0
#define MEU1_NDPU1_LA 0x45
#define MEU1_NDPU2_LA 0x46
#define MEU1_NDPU3_LA 0x47
#define MEU1_NDPU4_LA 0x48
#define MEU1_NDPU5_LA 0x49
#define MEU1_NDPU6_LA 0x4A
#define MEU1_PSU1_LA 0x4B
#define MEU1_PSU2_LA 0x4C
#define MEU1_RTR2_LA 0x22


/////////////////////////////////////////////////
//  Define the Physicals address of the devices
/////////////////////////////////////////////////
#define MEU1_ICUA_PH 0x01
#define MEU1_NDPU1_PH 0x07
#define MEU1_NDPU2_PH 0x08
#define MEU1_NDPU3_PH 0x09
#define MEU1_NDPU4_PH 0x0A
#define MEU1_NDPU5_PH 0x0B
#define MEU1_NDPU6_PH 0x0C
#define MEU1_PSU1_PH 0x0D
#define MEU1_PSU2_PH 0x0E
#define MEU1_RTR2_PH 0x0F

/////////////////////////////////////////////////
//  Define the baudrate and clock divisor for devices
/////////////////////////////////////////////////
#define MEU1_NDPU_BAUDRATE 0x64
#define RTR1_NDPU_PCTRL_RD 0x00
#define MEU1_PSU_BAUDRATE 0x0A
#define RTR1_PSU_PCTRL_RD 0x0A

// Define router addresses of the registers.
#define RTR_RTPMAP_PH_BASE 0x00000004
#define RTR_RTACTRL_PH_BASE 0x00000404
#define RTR_RTPMAP_LA_BASE 0x00000080
#define RTR_RTACTRL_LA_BASE 0x000007FC

#define RTR_PCTRLCFG_BASE 0x00000880
#define RTR_PCTRL_BASE 0x00000884


#endif
