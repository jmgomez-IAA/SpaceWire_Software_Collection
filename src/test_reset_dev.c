/*
  @file test_read_rmap.c
  @author Juan Manuel Gómez
  @brief Transmit a RMAP Read command to read a 4 Bytes register from target. 
  @details Demostrates how to configure GR718B using RMAP packets. The   
  configuration port is at SpaceWire Addr 0 and support RMAP
  target packet. The Register are memory mapped, and could be
  initalized through RMAP Write commands.
  @param address
  @param timeout
  @param verbose

  
  @comments
   More arguments: 
     - Target address path
     - target logical address
     - initiator address paths 
     - initiator logical address   
     - transactio identifier
    
  @example ./test_rmap  -a 0xA40 -t 30 -v
  @copyright jmgomez CSIC-IAA
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utility.h"
#include "star-dundee_types.h"
#include "star-api.h"
#include "cfg_api_router.h"
#include "cfg_api_mk2.h"
#include "cfg_api_mk2_types.h"
#include "cfg_api_brick_mk3.h"
#include "rmap_packet_library.h"

#define VERSION_INFO "star-system_test v2.0"

#include <errno.h>

#define STAR_INFINITE 30000

#define _TX_INTERFACE 1
#define _RX_INTERFACE 2
#define _PACKET_SIZE 64

#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4
#define _RX_BAUDRATE_MUL 2
#define _RX_BAUDRATE_DIV 4

#define _ADDRESS_PATH 2
#define _ADDRESS_PATH_SIZE 1


/* 
  Flags and variables requiered to manage
  the input parammeteres 
*/
static int verbose_flag;
static int reset_flag;
static int identify_flag;
static int address_flag;
static int timeout_flag;
static int output_flag;

static unsigned int verbose_level;
static unsigned int address_to_read;
static unsigned int op_timeout;
static unsigned int output_format;


int parse_parammeters (int paramc, char *const *paramv)
{
  int opt;

  if (paramc == 1)
  {
    fprintf (stderr,"One argument expected.\n");
    fprintf (stderr,"Sixtax: %s\n -r -i -v\n", paramv[0]);
    fprintf (stderr,"Reads a register from the device with and rmap read command.\n");
    fprintf (stderr,"\t-r \t Resets the device.\n");
    fprintf (stderr,"\t-i \t Identify the device.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\n\n");
    return -1;
  }

  while ((opt = getopt (paramc, paramv, "vri")) != -1)
  {
    switch (opt)
    {
      case 'v':  //verbose mode
        verbose_flag = 1;
        verbose_level = 1;
        if (verbose_flag == 1) fprintf (stderr,"Verbose mode enabled\n");
        break;

      case 'i':  //verbose mode
        identify_flag = 1;        
        if (verbose_flag == 1) fprintf (stderr,"Indentify device enabled\n");
        break;

      case 'r':  //verbose mode
        reset_flag = 1;        
        if (verbose_flag == 1)  fprintf (stderr,"Reset device.\n");
        break;

      case '?':
        if ( (optopt == 'a') || (optopt == 't') )
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;

      default:
        fprintf (stderr, "One argument expected.\n");
        return 1;
    }
  }

  for(; optind < paramc; optind++)
  {
    fprintf(stderr, "extra arguments: %s\n", paramv[optind]);
    return optind;
  } 

  return opt;
}


/******************************************************************/
/*                                                                */
/*****************************************************************/
int __cdecl main(int argc, char *argv[]){
  int opt;

  STAR_DEVICE_ID* devices;
  STAR_DEVICE_ID deviceId;
  U32 deviceCount = 0U, i;
  

  STAR_CHANNEL_MASK channelMask;
  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];

  unsigned int txChannelNumber, rxChannelNumber;
  STAR_CHANNEL_ID rxChannelId = 0U, txChannelId = 0U;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  verbose_flag    = 0;
  identify_flag   = 0;
  reset_flag      = 1;

  // Parse parammeteres
  if ( parse_parammeters (argc, argv) != -1){
    fprintf (stderr, "There are unkown parammeters,please check!!\n");
    return -1;
  }

  if (timeout_flag == 0)
  {
    fprintf (stderr, "Timeout for the reply reception to maximum.\n");
    op_timeout = STAR_INFINITE;
  }
  
  /* Get the list of devices of the specified type */
  devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED , &deviceCount);
  if (devices == NULL){
    fprintf (stderr, "STAR: No device found\n");	
    return 0;
  } 

  /* We are working with the first device, what if I have more? */
  deviceId = devices[0];	
  if (deviceId == STAR_DEVICE_UNKNOWN){
    fprintf (stderr, "STAR: Error dispositivo desconocido.\n");
    return 0;
  }

  /* Get hardware info*/
  CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
  CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);
  /* Display the hardware info*/
  if (verbose_level == 1)
  {
    fprintf(stderr, "\nVersion: %s\n", versionStr);
    fprintf(stderr, "\nBuildDate: %s\n", buildDateStr);	    
  }

  if (reset_flag == 1)
  {
    fprintf (stderr, "STAR: Reset the device\n");
    STAR_resetDevice(deviceId);
  }

  // Flashes the LEDS.
  if (identify_flag == 1)
  {
    if (CFG_MK2_identify(deviceId) == 0)
      {
        fprintf (stderr, "STAR: Unable to identify device.\n");
        return 0U;
      }
  }


  return 0;
}

