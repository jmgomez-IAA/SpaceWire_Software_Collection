/*
  @file test_receiv.c
  @author Juan Manuel Gómez
  @brief Receive files forever and dumps to stdio.
  @details 
  @param No parammeters needed.
  @example ./test_rmap  
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
#include "rmap_packet_library.h"

#include <sys/time.h>

clock_t start, finish;
clock_t GET_TIME()
  {
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);
    return tv.tv_sec * 1000000 + tv.tv_usec;
  }
#define TIME_DIVIDER    1000000


#define VERSION_INFO "star-system_test v2.0"

#include <errno.h>

#define STAR_INFINITE 30000

#define _TX_INTERFACE 1
#define _RX_INTERFACE 1
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
static int size_flag;
static int serial_flag;
static int timeout_flag;

static unsigned int verbose_level;
static unsigned int address_to_write;
static unsigned int packet_data_size;
static unsigned int op_serial;
static unsigned int op_timeout;

int parse_parammeters (int paramc, char *const *paramv)
{
  int opt;

  if (paramc == 1)
  {
    fprintf (stderr,"One argument expected.\n");
    fprintf (stderr,"Sixtax: %s -v -s serial_number -a targetAddress -b byteSize -p targetPath -t timeout\n", paramv[0]);
    fprintf (stderr,"Transmit a SpaceWire packet with random data.\n");    
    fprintf (stderr,"\t-s serial_number\t STAR-DUNDEE device serial number.\n");
    fprintf (stderr,"\t-t timeout\t milliseconds to wait for the reception of the reply.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\n\n");
    return -1;
  }

  while ((opt = getopt (paramc, paramv, "vris:t:")) != -1)
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
        if (verbose_flag == 1)  fprintf (stderr,"Reset device enabled %s\n", optarg);
        break;

      case 's': //Device Serial number
        serial_flag = 1;        
        unsigned long param_serial = strtoul(optarg, NULL, 10);
        op_serial = (unsigned int) param_serial;
        if (verbose_flag == 1) fprintf (stderr,"The argument supplied is %u\n", op_serial);
        break;

      case 't': //Time-out
        timeout_flag = 1;        
        unsigned long param_timeout = strtoul(optarg, NULL, 10);
        op_timeout = (unsigned int) param_timeout;
        if (verbose_flag == 1) fprintf (stderr,"The argument supplied is %u\n", op_timeout);
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

  STAR_DEVICE_ID* devices;
  STAR_DEVICE_ID deviceId;
  U32 deviceCount = 0U, i;
  unsigned int deviceActual = 0U;
  char *device_serial;  

  STAR_CHANNEL_MASK channelMask;
  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];

  unsigned int rxChannelNumber;
  STAR_CHANNEL_ID rxChannelId = 0U;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  verbose_flag    = 0;
  verbose_level   = 0;
  identify_flag   = 0;
  reset_flag      = 0;
  timeout_flag    = 0;
  serial_flag     = 0;

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

  /***************************************************************/
  /*        Configuration                                        */
  /*                                                             */
  /* Channel 1 = Transmit interface                              */
  /* Channel 2 = Receive  interface                              */
  /* BaudRate  = 100 Mbps (100*2/4)*2                            */
  /* Packet Size = 1 KB                                          */
  /***************************************************************/

  rxChannelNumber = _RX_INTERFACE;
  clockRateParams.multiplier = _TX_BAUDRATE_MUL;
  clockRateParams.divisor = _TX_BAUDRATE_DIV;
  
  /* Get the list of devices of the specified type */
  devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED , &deviceCount);
  if (devices == NULL){
    fprintf (stderr, "No device found\n");	
    return 0;
  } 

  if (serial_flag == 1)
  {
    /* Search the device with the serial number specified. */
    for (deviceActual = 0; deviceActual < deviceCount; deviceActual ++)
    {      

      if (devices[deviceActual] == STAR_DEVICE_UNKNOWN){
        fprintf (stderr, "Error dispositivo desconocido.\n");
      }
      else
      {
        /* Get device Serial Number */
        device_serial = STAR_getDeviceSerialNumber(devices[deviceActual]);
        unsigned long dev_serial = strtoul(device_serial, NULL, 10);
        if (verbose_level == 1)
        {
          fprintf (stderr, "Device %d serial number is %s.\n", deviceActual, device_serial);
        }

        if (dev_serial == op_serial)
        {
          deviceId = devices[deviceActual];

          if (verbose_level == 1)
            {
              fprintf (stderr, "Select device %d serial number is %s.\n", deviceActual, device_serial);
            }
        }

        STAR_destroyString(device_serial);
      }      
    }
  }
  else
  {
    /* We are working with the first device, what if I have more? */  
    deviceId = devices[0];
  }

  /* Get hardware info*/
  CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
  CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);
  /* Display the hardware info*/
  if (verbose_level == 1)
    {
      fprintf(stderr, "Version: %s\n", versionStr);
      fprintf(stderr, "BuildDate: %s\n", buildDateStr);	
    }

  // Flashes the LEDS.
  if (identify_flag == 1)
  {
    if (CFG_MK2_identify(deviceId) == 0)
    {
     fprintf (stderr, "ERROR: Unable to identify device.\n");
     return 0U;
    }
  }

  /* Get the channels present on the device  */
  /* We need at least 2 Channels (0x00000007) */
  channelMask = STAR_getDeviceChannels(deviceId);
  if (channelMask != 7U)
  {
    fprintf (stderr, "The device does not have channel 1 and 2 available.\n");
    return 0U;
  }

  int status_link = 0;
  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, rxChannelNumber, clockRateParams);	
  if (status_link == 0){
    fprintf (stderr, "Error setting the Tx link speed.\n");	
  }

  /* Open channel 1 for Receive Operation*/
  rxChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_IN,
					      rxChannelNumber, TRUE);
  if (rxChannelNumber == 0U)
  {
    fprintf (stderr, "ERROR: Unable to open channel 1 to receive.\n");
    return 0;
  }  

  if (verbose_level == 1)
  {
    fprintf (stderr, "Channels Opened.\n");  
  }
  
	
  /*****************************************************************/
  /*    Create the RX operation                                    */
  /*                                                               */
  /*****************************************************************/
  unsigned long byteSize = 0;

  byteSize = _PACKET_SIZE;
  STAR_TRANSFER_STATUS rxStatus;
  STAR_TRANSFER_OPERATION *pRxTransferOp = NULL;
  STAR_STREAM_ITEM *pTxStreamItem = NULL;
   
  /* Create the receive operations. */
  pRxTransferOp = STAR_createRxOperation(1, STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
  {
    fprintf (stderr, "ERROR: Unable to create receive operation\n");
    return 0;
  }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /* if an operation there is fail and we already have 20 packets*/
  /* close the app.                                              */
  /***************************************************************/
  int counter= 0;
  while(counter < 20 || (rxStatus == STAR_TRANSFER_STATUS_COMPLETE))
  {
    /* Submit the receive operation */
    if (STAR_submitTransferOperation(rxChannelId, pRxTransferOp) == 0)
    {
      fprintf (stderr, "ERROR occurred during receive.  Test Tfailed.\n");
      return 0;
    }

    /* Wait on the receive operation completing */
    rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
                                                      op_timeout);
    if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      fprintf (stderr, "ERROR occurred during receive.  Test failed.\n");
    }

    // Store the received packet in the file.
    /* Get the number of traffic items received */
    unsigned int rxPacketCount = STAR_getTransferItemCount(pRxTransferOp);
    if (rxPacketCount == 0)
    {
      fprintf (stderr, "No packets received in this operation .\n");
    }
    else
    {
      /* For each traffic item received */
      int i = 0;
      for (i = 0U; i < rxPacketCount; i++)
      {
        /* Get the packet */
        STAR_STREAM_ITEM *pRxStreamItem = STAR_getTransferItem(pRxTransferOp, i);
        if ((pRxStreamItem == NULL) || (pRxStreamItem->item == NULL) ||
            (pRxStreamItem->itemType != STAR_STREAM_ITEM_TYPE_SPACEWIRE_PACKET) )
        {
          fprintf (stderr, "ERROR received an unexpected traffic type, or empty traffic item in item %lu\n", i);
        }
        else
        {

          unsigned int streamDataSize = 0;
          // Get the Packet Data and size from the stream Stream
          unsigned char* pPacketBufferData = STAR_getPacketData((STAR_SPACEWIRE_PACKET *) pRxStreamItem->item,
                                             &streamDataSize);
          // Received packets will not have an address structure set. 
          //STAR_SPACEWIRE_ADDRESS *pStreamItemAddress =  STAR_getPacketAddress ( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);

          printf("\n");

          fprintf (stderr, "Packet Number:\t%d\n", counter);
          int dat_iter = 0;
          for ( dat_iter= 0; dat_iter < streamDataSize; ++dat_iter){

            fprintf (stderr, "\t0x%x" ,pPacketBufferData[dat_iter]);
            if (! ((dat_iter+1) % 8) )
            {
              fprintf (stderr, "\n");
            }
          }

          fflush(stdout);

          //          STAR_destroyAddress(pStreamItemAddress);
          STAR_destroyPacketData(pPacketBufferData);
        }
      }
    }
 
    counter++;
  }

  /****************************************************************/
  /*    Free the resource                                         */
  /*                                                              */
  /****************************************************************/

  /* Dispose of the transfer operations */  
  if (pRxTransferOp != NULL)
  {
    STAR_disposeTransferOperation(pRxTransferOp);
  }

  /* Close the channels */
  if (rxChannelId != 0U)
  {
    STAR_closeChannel(rxChannelId);
  }

  return 0;
}





