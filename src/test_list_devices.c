/*
  @file test_list_devices.c
  @author Juan Manuel Gómez
  @brief Check for SpaceWire devices connected and list it ids.
  @details 
  @remark 
  @todo 
  @param No parammeters needed.
  @example 
  @copyright jmgomez CSIC-IAA
 */

#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "star-dundee_types.h"
#include "star-api.h"
#include "cfg_api_router.h"
#include "cfg_api_mk2.h"
#include "cfg_api_mk2_types.h"
#include "cfg_api_brick_mk3.h"

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

#define _ADDRESS_PATH 0xFE
#define _ADDRESS_PATH_SIZE 1


/* 
  Flags and variables requiered to manage
  the input parammeteres 
*/
static int verbose_flag;
static int reset_flag;
static int identify_flag;
static unsigned int verbose_level;

int parse_parammeters (int paramc, char *const *paramv)
{
  int opt;

/*
  if (paramc == 1)
  {
    fprintf (stderr,"Argument expected.\n");
    fprintf (stderr,"Sintax: %s\t -v -r -i\n", paramv[0]);
    fprintf (stderr,"Check for SpaceWire devices connected.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\t-r reset the devices.\n");
    fprintf (stderr,"\t-i identify the devices.\n");
    fprintf (stderr,"\n\n");
    return -1;
  }
*/
  while ((opt = getopt (paramc, paramv, "vri")) != -1)
  {
    switch (opt)
    {
      case 'v':  //verbose mode
        verbose_flag = 1;
        verbose_level = 1;
        if (verbose_flag == 1) fprintf (stderr,"Verbose mode enabled\n");
        break;

      case 'i':  //Identify flags
        identify_flag = 1;        
        if (verbose_flag == 1) fprintf (stderr,"Indentify device enabled\n");
        break;

      case 'r':  //reset device
        reset_flag = 1;        
        if (verbose_flag == 1)  fprintf (stderr,"Reset device enabled\n");
        break;

      default:
        fprintf (stderr, "Sintax error argument expected.\n");
        return 1;
    }
  }

  for(; optind < paramc; optind++)
  {
    fprintf(stderr, "Extra arguments detected: %s\n", paramv[optind]);
    return optind;
  } 

  return opt;
}


/******************************************************************/
/*                                                                */
/*****************************************************************/
int __cdecl main(int argc, char *argv[]){

  STAR_DEVICE_ID* devices;
  STAR_DEVICE_ID deviceId;
  U32 deviceCount = 0U, deviceActual = 0U;

  STAR_CHANNEL_MASK channelMask;
  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];
  char device_name_list[10][64];
  char device_serial_list[10][64];
  char *device_name;
  char *device_serial;

  verbose_flag    = 0;  
  identify_flag   = 0;
  reset_flag      = 0;

  verbose_level   = 0;
  

    // Parse parammeteres
  if ( parse_parammeters (argc, argv) != -1){
    fprintf (stderr, "There are unkown parammeters,please check!!\n");
    return -1;
  }


  /***************************************************************/
  /*        Detect SpaceWire devices                             */
  /*                                                             */
  /***************************************************************/
  
  /* Get the list of devices of the specified type */
  devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED , &deviceCount);
  if (devices == NULL){
    fprintf (stderr, "No device found\n");	
    return 0;
  } 

  for (deviceActual = 0; deviceActual < deviceCount; deviceActual ++)
  {
    /* We are working with the first device, what if I have more? */
    deviceId = devices[deviceActual];
    if (deviceId == STAR_DEVICE_UNKNOWN)
    {
      fprintf (stderr, "Error dispositivo desconocido.\n");    
    }
    
    device_name = STAR_getDeviceName(deviceId);
    if (device_name == NULL)
    {
      fprintf (stderr, "Device name error.\n");
    }

    /* Get hardware info*/
    CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
    CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);

    /* Get device Serial Number */
    device_serial = STAR_getDeviceSerialNumber(deviceId);

    /* Display the hardware info*/
    if (verbose_level == 1)
    {
      fprintf (stdout, "Version: %s", versionStr);
      fprintf (stdout, "BuildDate: %s", buildDateStr);  
    }

    if (identify_flag == 1)
    {
      // Flashes the LEDS.
      if (CFG_MK2_identify(deviceId) == 0)      
        fprintf (stderr, "\nERROR: Unable to identify device");        
      

    }

    if (reset_flag == 1)
    {
      if (STAR_resetDevice(deviceId) == 0)
        fprintf (stderr, "STAR: Reset the device\n");
    }


    strcpy(device_name_list[deviceActual], device_name);
    strcpy(device_serial_list[deviceActual], device_serial);

    STAR_destroyString(device_name);
    STAR_destroyString(device_serial);
  }

  if (deviceCount > 0)
  {
    fprintf(stdout, "{\"Status\" : \"Success\", \"DeviceNumber\":%d, \"DeviceList\" : [",  deviceCount);
      for (deviceActual = 0; deviceActual < deviceCount; deviceActual ++)
          fprintf(stdout, "{\"DeviceId\":%d,\"DeviceSerial\":\"%s\"}",  deviceActual, device_serial_list[deviceActual]);
        
    fprintf(stdout, "]}\n");
  }
  else
  {
    fprintf(stdout, "{\"Status\" : Fail}\n");
  }

  return 0;
}





