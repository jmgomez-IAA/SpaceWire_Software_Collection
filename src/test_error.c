/*
  @file test_send.c
  @author Juan Manuel Gómez
  @brief Transmit a packet with the Star-Dundee MK3 Brick.
  @details Transmit a packet using the Star-Dundee MK3 Brick.
  @remark .
  @todo 
  @param 
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
#define _PACKET_SIZE 64

#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4
#define _RX_BAUDRATE_MUL 2
#define _RX_BAUDRATE_DIV 4

#define _ADDRESS_PATH 0xFE
#define _ADDRESS_PATH_SIZE 1

/* 
  Flags and variables requiered to manage the input parammeteres 
*/
static int verbose_flag;
static int reset_flag;
static int identify_flag;
static int address_flag;
static int size_flag;
static int timeout_flag;
static int serial_flag;

static unsigned int verbose_level;
static unsigned int address_to_write;
static unsigned int packet_data_size;
static unsigned int op_timeout;
static unsigned int op_serial;

int parse_parammeters (int paramc, char *const *paramv)
{
  int opt;

  if (paramc == 1)
  {
    fprintf (stderr,"One argument expected.\n");
    fprintf (stderr,"Sintax: %s\n-a targetAddress -b byteSize -p targetPath -t timeout -v\n", paramv[0]);
    fprintf (stderr,"Write a register from the device with and rmap read command.\n");
    fprintf (stderr,"\t-a targetAddress\t address in hexadecimal to write.\n");
    fprintf (stderr,"\t-b dataSize\t packet data size n bytes.\n");
    fprintf (stderr,"\t-p addressPath\t target address to path from the trnsmiter.\n");
    fprintf (stderr,"\t-t timeout\t milliseconds to wait for the reception of the reply.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\n\n");
    return -1;
  }

  while ((opt = getopt (paramc, paramv, "vris:b:a:p:t:")) != -1)
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

      case 'b':
        size_flag = 1;
        unsigned long param_data = strtoul(optarg, NULL, 10);
        packet_data_size = (unsigned int) param_data;
        if (verbose_flag == 1) fprintf (stderr,"The data size supplied is %d\n", packet_data_size);
        break;

      case 'a':  //address supplied
        address_flag = 1;        
        unsigned long param_addr = strtoul(optarg, NULL, 16);
        address_to_write = (unsigned int) param_addr;
        if (verbose_flag == 1) fprintf (stderr,"The target address supplied is %u\n", address_to_write);
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
  U32 deviceCount = 0U, i, deviceActual;
  char *device_serial; 

  STAR_CHANNEL_MASK channelMask;
  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];

  unsigned int txChannelNumber, rxChannelNumber;
  STAR_CHANNEL_ID rxChannelId = 0U, txChannelId = 0U;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;


  verbose_flag    = 0;
  address_flag    = 0;
  size_flag       = 0;
  timeout_flag    = 0;
  identify_flag   = 0;
  reset_flag      = 0;

  verbose_level   = 0;
  address_to_write = 0;
  packet_data_size = 0;


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

  if (size_flag == 0)
  {
    fprintf (stderr, "Packet size set to default %d bytes.\n", _PACKET_SIZE);
    packet_data_size = _PACKET_SIZE;
  }

  if (address_flag == 0)
  {
    fprintf (stderr, "Target Address set to default 0xFE.\n" );
    address_to_write = _ADDRESS_PATH;
  }


  /***************************************************************/
  /*        Configuration                                        */
  /*                                                             */
  /* Channel 1 = Transmit interface                              */
  /* Channel 2 = Receive  interface                              */
  /* BaudRate  = 100 Mbps (100*2/4)*2                            */
  /* Packet Size = 1 KB                                          */
  /***************************************************************/

  txChannelNumber = _TX_INTERFACE;
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
        fprintf (stderr, "\nERROR: Unable to identify device");
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
  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, txChannelNumber, clockRateParams); 
  if (status_link == 0){
    fprintf (stderr, "Error setting the Tx link speed.\n"); 
  }

  /* Open channel 1 for Transmit*/
  txChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT,
                txChannelNumber, TRUE);
  if (txChannelNumber == 0U)
    {
      fprintf (stderr, "\nERROR: Unable to open TX channel 1\n");
      return 0;
    }

  if (verbose_level == 1)
    {
      fprintf (stderr, "Channels Opened.\n"); 
    } 
  
  /*****************************************************************/
  /*    Allocate memory for the transmit buffer and construct      */
  /*    the packet to transmit.                                    */
  /*    #define _ADDRESS_PATH 1                                    */
  /*    #define _ADDRESS_PATH_SIZE 1                               */
  /*****************************************************************/
  unsigned long byteSize = 0;
  char *pTxBuffer = NULL;
  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL;
  STAR_STREAM_ITEM *pTxStreamItem[3];
  STAR_STREAM_ITEM *packetStart, *packetEnd;
  STAR_STREAM_ITEM *disconnectErrorItem, *parityErrorItem;
  unsigned char newPath[128];
  unsigned int pathLen = 0;
  STAR_SPACEWIRE_ADDRESS *pAddressPath = NULL;

  byteSize = packet_data_size;
  pathLen = _ADDRESS_PATH_SIZE;
  newPath[0] = address_to_write;
  newPath[1] = 2;

  /* Allocate memory for the transmit buffer */
  pTxBuffer = (char *)calloc(1U, byteSize);
  if (pTxBuffer == NULL)
    {
      fprintf (stderr, "\nERROR: Unable to allocate memory for transmit buffer");
      return 0;
    }
  /*Initialize with data*/
    pTxBuffer[0] = address_to_write;
  for(i=1; i< byteSize; ++i){
    pTxBuffer[i] = i;
  }

  /* Create a SpaceWire address from the path */
  //pAddressPath = STAR_createAddress(newPath, pathLen);

  /* Create the packet to be transmitted */
  //pTxStreamItem = STAR_createPacket(NULL, (U8 *)pTxBuffer, byteSize,
  //          STAR_EOP_TYPE_EOP);
  unsigned int packetStartLength = byteSize /2;

  packetStart = STAR_createDataChunk(pTxBuffer,
                                    packetStartLength, 1, STAR_EOP_TYPE_NONE);
  packetEnd = STAR_createDataChunk(
                                    pTxBuffer + packetStartLength,
                                    byteSize - packetStartLength, 0, STAR_EOP_TYPE_EOP);
  
  disconnectErrorItem = STAR_createErrorInData(
                                    STAR_ERROR_IN_DATA_DISCONNECT);
  
  parityErrorItem = STAR_createErrorInData(STAR_ERROR_IN_DATA_PARITY);
  /* Make packet with from stream items with disconnect error */
  pTxStreamItem[0] = packetStart;
  pTxStreamItem[1] = disconnectErrorItem;
  pTxStreamItem[2] = packetEnd;

  if (pTxStreamItem == NULL)
    {
      fprintf (stderr, "\nERROR: Unable to create the packet to be transmitted");
      return 0;
    }

  /* Print the Packet. Diabled for Packets bigger thant 64B */
  //if (verbose_level == 1)
  //  {
      //dumpPacket((STAR_SPACEWIRE_PACKET *)pTxStreamItem->item);
   // }

  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(pTxStreamItem, 3U);
  if (pTxTransferOp == NULL)
    {
      fprintf (stderr, "\nERROR: Unable to create the transfer operation to be transmitted");
      return 0;
    }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /***************************************************************/

  /* Submit the transmit operation */
  if (STAR_submitTransferOperation(txChannelId, pTxTransferOp) == 0)
    {
      printf("\nERROR occurred during transmit.  Test failed.\n");
      return 0;
    }

  //Wait the operations to finish.
  STAR_TRANSFER_STATUS rxStatus, txStatus;

  /* Wait on the transmit operation completing */
  txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
                STAR_INFINITE);
  if(txStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      printf("\nERROR occurred during transmit.  Test failed.\n");
      return 0;
    }

  /****************************************************************/
  /*    Check the Results                                         */
  /*                                                              */
  /****************************************************************/
    fprintf(stdout, "{\"%d\" : Success}\n", pTxBuffer[2]);
  
  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL)
    {
      STAR_disposeTransferOperation(pTxTransferOp);
    }
  
  /* Destroy the packet transmitted */
  if (pTxStreamItem != NULL)
    {
      STAR_destroyStreamItem(pTxStreamItem[0]);
      STAR_destroyStreamItem(pTxStreamItem[1]);
      STAR_destroyStreamItem(pTxStreamItem[2]);
    }

  /* Free the transmit buffer */
  if (pTxBuffer != NULL)
    {
      free(pTxBuffer);
    }

  /* Free the address path */
  if (pAddressPath != NULL)
    {
      STAR_destroyAddress(pAddressPath);
    }

  /* Close the channels */
  if (rxChannelId != 0U)
    {
      STAR_closeChannel(rxChannelId);
    }

  if (txChannelId != 0U)
    {
      STAR_closeChannel(txChannelId);
    }

  return 0;
}
