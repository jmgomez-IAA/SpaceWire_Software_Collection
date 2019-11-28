/*
  @file test_rmap.c
  @author Juan Manuel Gómez
  @brief Demostration RMAP packet to configure GR718 
  @details 
  @param address Address of the register to load.
  @param value
  #return The value of the register or the error code in case of problems.
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

#define VERSION_INFO "star-system_test v2.0"

#include <errno.h>


/**
* Default values for the configuration
* ====================================
*/
// Operation timeout.
#define STAR_INFINITE 30000

// Interface operation.
#define _TX_INTERFACE 1
#define _RX_INTERFACE 2

// Default packet size.
#define _PACKET_SIZE 64

// Interface Baudrate configuraiton: 100 Mbps
#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4
#define _RX_BAUDRATE_MUL 2
#define _RX_BAUDRATE_DIV 4

// Default address path.
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
static int data_flag;
static int timeout_flag;

static unsigned int verbose_level;
static unsigned int address_to_write;
static unsigned int data_to_write;
static unsigned int op_timeout;
static unsigned int output_format;


int parse_parammeters (int paramc, char *const *paramv)
{
  int opt;

  if (paramc == 1)
  {
    fprintf (stderr,"One argument expected.\n");
    fprintf (stderr,"Sixtax: %s\n -a address -t timeout -v\n", paramv[0]);
    fprintf (stderr,"Write a register from the device with and rmap read command.\n");
    fprintf (stderr,"\t-a address\t address in hexadecimal to write.\n");
    fprintf (stderr,"\t-d data\t address in hexadecimal to write.\n");
    fprintf (stderr,"\t-t timeout\t milliseconds to wait for the reception of the reply.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\n\n");
    return -1;
  }

  while ((opt = getopt (paramc, paramv, "via:d:t:o:")) != -1)
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
        if (verbose_flag == 1)  fprintf (stderr,"Reset device enabled %s\n", optarg);
        break;

      case 'd':
        data_flag = 1;
        unsigned long param_data = strtoul(optarg, NULL, 16);
        data_to_write = (unsigned int) param_data;
        if (verbose_flag == 1) fprintf (stderr,"The argument supplied is %u\n", data_to_write);
        break;

      case 'a':  //address supplied
        address_flag = 1;        
        unsigned long param_addr = strtoul(optarg, NULL, 16);
        address_to_write = (unsigned int) param_addr;
        if (verbose_flag == 1) fprintf (stderr,"The argument supplied is %u\n", address_to_write);
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

  STAR_CHANNEL_MASK channelMask;
  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];

  unsigned int txChannelNumber, rxChannelNumber;
  STAR_CHANNEL_ID rxChannelId = 0U, txChannelId = 0U;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  /**
   * Parse arguments.
  */
  verbose_flag    = 0;
  address_flag    = 0;
  data_flag       = 0;
  timeout_flag    = 0;
  identify_flag   = 0;
  reset_flag      = 0;

  // Parse parammeteres
  if ( parse_parammeters (argc, argv) != -1){
    fprintf (stderr, "There are unkown parammeters,please check!!\n");
    return -1;
  }

  if (address_flag == 0 || data_flag == 0)
  {
    fprintf (stderr, "The address and data to write from the target device are required.\n");
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
  txChannelNumber = _TX_INTERFACE;
  rxChannelNumber = _RX_INTERFACE;
  clockRateParams.multiplier = _TX_BAUDRATE_MUL;
  clockRateParams.divisor = _TX_BAUDRATE_DIV;
  

  /* Get the list of devices of the specified type */
  devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED , &deviceCount);
  if (devices == NULL){
    fprintf (stderr, "No device found\n");
    return 0;
  } 

  /* We are working with the first device, what if I have more? */
  deviceId = devices[0];	
  if (deviceId == STAR_DEVICE_UNKNOWN){
    fprintf (stderr, "Error dispositivo desconocido.\n");
    return 0;
  }

  /* Get hardware info*/
  CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
  CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);
  /* Display the hardware info*/
  if (verbose_level == 1)
    {  
      printf("Version: %s\n", versionStr);
      printf("BuildDate: %s\n", buildDateStr);	
    }

  // Flashes the LEDS.
  if (CFG_MK2_identify(deviceId) == 0)
    {
      fprintf (stderr, "ERROR: Unable to identify device\n");
      return 0U;
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

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, rxChannelNumber, clockRateParams );
  if (status_link == 0){
    fprintf (stderr, "Error setting the RX link speed.\n");	
  }

  /* Open channel 1 for Transmit*/
  txChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT,
					      txChannelNumber, TRUE);
  if (txChannelNumber == 0U)
    {
      fprintf (stderr, "\nERROR: Unable to open TX channel 1\n");
      return 0;
    }
  /* Open channel 2 for Receipt*/
  rxChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_IN,
					      rxChannelNumber, TRUE);
  if (rxChannelId == 0U)
    {
      fprintf (stderr, "\nERROR: Unable to open RX channel 2\n");
      return 0;
    }

  if (verbose_level == 1)
    {
      fprintf (stderr, "Channels Opened.\n");	
    }
  /*****************************************************************/
  /*    Allocate memory for the transmit buffer and construct      */
  /*    the packet to transmit.                                    */
  /*****************************************************************/
  unsigned long byteSize = 0;

  void *pFillPacket, *pBuildPacket;
  unsigned long fillPacketLenCalculated, fillPacketLen;  
  unsigned long buildPacketLen;
  U8 pTarget[] = {0,254};
  U8 pReply[] = {254};
  U8 pData[4];
  char status;


  //memcpy(pData, U8*) data_to_write, 4);



  pData[0] = (data_to_write & 0xFF000000) >> 24;
  pData[1] = (data_to_write & 0x00FF0000) >> 16;
  pData[2] = (data_to_write & 0x0000FF00) >> 8;
  pData[3] = data_to_write & 0x000000FF;
  byteSize = _PACKET_SIZE;
  
  /* Calculate the length of a 
    **write command packet**, with 4 bytes of data 
  */
  fillPacketLenCalculated = RMAP_CalculateWriteCommandPacketLength(2, 1, 4,
								   1);

  if (verbose_level == 1)
    {
      printf("The write command packet will have a length of %ld bytes\n",
	           fillPacketLenCalculated);
    }

  /* Allocate memory for the packet */
  pFillPacket = malloc(fillPacketLenCalculated);
  if (!pFillPacket)
    {
      fprintf (stderr, "Couldn't allocate the memory for the command packet\n");
      return;
    }

  /* Fill the memory with the packet */  
  status = RMAP_FillWriteCommandPacket(pTarget, 2, pReply, 1, 1, 1, 0, 0x00,
				       0, address_to_write, 0, pData, 4, &fillPacketLen, NULL, 1, (U8 *)pFillPacket,
				       fillPacketLenCalculated);
  if (!status)
    {
      fprintf (stderr, "Couldn't fill the write command packet\n");
      free(pFillPacket);
      return;
    }

  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL, *pRxTransferOp = NULL;
  STAR_STREAM_ITEM *pTxStreamItem = NULL;
   
  /* Create the receive operations. */
  pRxTransferOp = STAR_createRxOperation(1, STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
    {
      fprintf (stderr, "ERROR: Unable to create receive operation\n");
      return 0;
    }

  pTxStreamItem = STAR_createPacket(NULL, (U8 *)pFillPacket, fillPacketLen,
				    STAR_EOP_TYPE_EOP);

  if (pTxStreamItem == NULL)
    {
      fprintf (stderr, "ERROR: Unable to create the packet to be transmitted\n");
      return 0;
    }

  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(&pTxStreamItem, 1U);
  if (pTxTransferOp == NULL)
    {
      fprintf (stderr, "ERROR: Unable to create the transfer operation to be transmitted\n");
      return 0;
    }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /***************************************************************/

  /* Submit the receive operation */
  /* Submit the transmit operation */
  if (STAR_submitTransferOperation(txChannelId, pTxTransferOp) == 0)
    {
      printf("ERROR occurred during transmit.\n\t\tTest failed.\n");
      return 0;
    }

  //Wait the operations to finish.
  STAR_TRANSFER_STATUS rxStatus, txStatus;

  /* Wait on the transmit operation completing */
  txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
						    op_timeout);
  if(txStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      printf("ERROR occurred during transmit.\nTest failed.\n");
      return 0;
    }

if (STAR_submitTransferOperation(txChannelId, pRxTransferOp) == 0)
    {
      printf("ERROR occurred during receive. \nTest Tfailed.\n");
      return 0;
    }

  /* Wait on the receive operation completing */
  rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
						    op_timeout);
  if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      printf("ERROR occurred during receive.\nTest failed.\n");
      return 0;
    }

  /****************************************************************/
  /*    Parse the response                                        */
  /*                                                              */
  /****************************************************************/
  unsigned long register_data_value = 0;
  unsigned long packet_iterator;
  unsigned int rxPacketCount;
  STAR_STREAM_ITEM *pRxStreamItem;
  STAR_TRANSFER_OPERATION * pTransferOp;
  unsigned char* pRxStreamData;
  unsigned int pRxStreamDataSize = 0;

  unsigned char* rply_data;
  unsigned int rply_data_length;

  RMAP_PACKET packetStruct;
  RMAP_STATUS conv_status;
  RMAP_STATUS rply_status;

  /* Get the number of traffic items received */
  rxPacketCount = STAR_getTransferItemCount((STAR_TRANSFER_OPERATION *)  pRxTransferOp);
  
  if (rxPacketCount != 0)
  {
    // Search for the reply packet.
      for (packet_iterator = 0U; packet_iterator < rxPacketCount; packet_iterator++)
      {
        /* Get the packet */
        pRxStreamItem = STAR_getTransferItem((STAR_TRANSFER_OPERATION *)  pRxTransferOp,
                                             packet_iterator);

        if ( pRxStreamItem->itemType == STAR_STREAM_ITEM_TYPE_SPACEWIRE_PACKET) 
        {
          
          if (verbose_flag == 1) dumpPacket( (STAR_SPACEWIRE_PACKET *) pRxStreamItem->item);
          // Get Data   
          pRxStreamDataSize = 0;          
          pRxStreamData = STAR_getPacketData( (STAR_SPACEWIRE_PACKET *) pRxStreamItem->item,
                                                &pRxStreamDataSize);

          // Check packet structure and update it on the packetStruct variable. The data is dependant of the
          // memory of  pRxStreamData.
          conv_status = RMAP_CheckPacketValid   (   (void *) pRxStreamData,
                                                    pRxStreamDataSize,
                                                    &packetStruct,
                                                    1 );                              

          if (conv_status == RMAP_SUCCESS) {
            rply_status = RMAP_GetStatus( &packetStruct);
            RMAP_PACKET_TYPE type = RMAP_GetPacketType(&packetStruct);

            if (verbose_level == 1)
            {            
              display_packet_type(type);
              display_packet_status(rply_status);
            }

            if (rply_status == RMAP_SUCCESS)
            {
              U16 transactionid = RMAP_GetTransactionID( &packetStruct);      
              fprintf(stdout, "{\"%d\" : Success}\n", transactionid);
            }
            else
            {
              fprintf(stderr, "STAR: The RMAP packet received with error code %d.\n", rply_status);
            }            
          }
          else
          {
            fprintf(stderr, "STAR: The packet is %sa valid RMAP packet.\n", (conv_status == RMAP_SUCCESS) ? "" : "not ");            
          }

          STAR_destroyPacketData(pRxStreamData);          
        }
        else{
          fprintf(stderr, "STAR: skip packet %d.\n", packet_iterator);     
        }
      }
  }
  else
  {
    fprintf(stderr, "STAR: No reply received.\n");
  }

  /****************************************************************/
  /*    Free the resource                                         */
  /*                                                              */
  /****************************************************************/

  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL)
    {
      STAR_disposeTransferOperation(pTxTransferOp);
    }

  if (pRxTransferOp != NULL)
    {
      STAR_disposeTransferOperation(pRxTransferOp);
    }

  if (pFillPacket != NULL){
    RMAP_FreeBuffer(pFillPacket);
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
