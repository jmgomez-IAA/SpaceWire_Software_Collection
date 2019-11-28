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
    fprintf (stderr,"Sixtax: %s\n -a address -t timeout -v\n", paramv[0]);
    fprintf (stderr,"Reads a register from the device with and rmap read command.\n");
    fprintf (stderr,"\t-a address\t address in hexadecimal to read.\n");
    fprintf (stderr,"\t-t timeout\t milliseconds to wait for the reception of the reply.\n");
    fprintf (stderr,"\t-v\t\t verbose mode.");
    fprintf (stderr,"\n\n");
    return -1;
  }

  while ((opt = getopt (paramc, paramv, "vria:t:o:")) != -1)
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

      case 'o':
        output_flag = 1;
        if (verbose_flag == 1) fprintf (stderr,"Output format selected as %s\n", optarg);
        break;

      case 'a':  //address supplied
        address_flag = 1;        
        unsigned long param_addr = strtoul(optarg, NULL, 16);
        address_to_read = (unsigned int) param_addr;
        if (verbose_flag == 1) fprintf (stderr,"The argument supplied is %u\n", address_to_read);
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
  address_flag    = 0;
  timeout_flag    = 0;
  identify_flag   = 0;
  reset_flag      = 0;

  // Parse parammeteres
  if ( parse_parammeters (argc, argv) != -1){
    fprintf (stderr, "There are unkown parammeters,please check!!\n");
    return -1;
  }

  if (address_flag == 0)
  {
    fprintf (stderr, "The address to read from the target device is required.\n");
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
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);	    
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

  if (reset_flag == 1)
  {
      STAR_resetDevice  ( deviceId);
      return 0U;
  }

  /* Get the channels present on the device  */
  /* We need at least 2 Channels (0x00000007) */
  channelMask = STAR_getDeviceChannels(deviceId);
  if (channelMask != 7U)
    {
      fprintf (stderr, "STAR: The device does not have channel 1 and 2 available.\n");
      return 0U;
    }

  int status_link = 0;
  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, txChannelNumber, clockRateParams);	
  if (status_link == 0){
    fprintf (stderr, "STAR: Error setting the Tx link speed.\n");	
  }

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, rxChannelNumber, clockRateParams );
  if (status_link == 0){
    fprintf (stderr, "STAR: Error setting the RX link speed.\n");	
  }

  /* Open channel 1 for Transmit and receive*/
  txChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT,
					      txChannelNumber, TRUE);
  if (txChannelNumber == 0U)
    {
      fprintf (stderr, "STAR: Unable to open TX channel 1\n");
      return 0;
    }

  /* Open channel 2 for Receipt*/
  rxChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_IN,
					      rxChannelNumber, TRUE);
  if (rxChannelId == 0U)
    {
      fprintf (stderr, "STAR: Unable to open RX channel 2\n");
      return 0;
    }

  if (verbose_flag == 1) fprintf (stderr, "STAR: Channels Opened.\n");	
	
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
  U8 pData[] = {0x00, 0x14, 0x02, 0x2E};
  char status;

  byteSize = _PACKET_SIZE;

  /*
    Calculate the length of a rmap read command packet
    with 4 bytes of data 
    */
  fillPacketLenCalculated = RMAP_CalculateReadCommandPacketLength(2, 1,1);
  if (verbose_flag == 1) 
  {
      fprintf (stderr, "STAR: The Read command packet will have a length of %ld bytes\n", 
          fillPacketLenCalculated);
  }

  /* Allocate memory for the packet */
  pFillPacket = malloc(fillPacketLenCalculated);
  if (!pFillPacket)
    {
      fprintf (stderr, "STAR: Couldn't allocate the memory for the command packet\n");
      return;
    }    

  status = RMAP_FillReadCommandPacket(pTarget, 2, pReply, 1, 0, 0x00,
				      0, address_to_read, 0, 4, &fillPacketLen, NULL, 1, (U8 *)pFillPacket,
				       fillPacketLenCalculated);
  if (!status)
    {
      fprintf (stderr, "STAR: Couldn't fill the write command packet\n");
      free(pFillPacket);
      return;
    }

  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL, *pRxTransferOp = NULL;
  STAR_STREAM_ITEM *pTxStreamItem = NULL;
   
  /* Create the receive operations. */
  pRxTransferOp = STAR_createRxOperation(1, STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
    {
      fprintf (stderr, "STAR: Unable to create receive operation\n");
      return 0;
    }

  pTxStreamItem = STAR_createPacket(NULL, (U8 *)pFillPacket, fillPacketLen,
				    STAR_EOP_TYPE_EOP);

  if (pTxStreamItem == NULL)
    {
      fprintf (stderr, "STAR: Unable to create the packet to be transmitted\n");
      return 0;
    }

  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(&pTxStreamItem, 1U);
  if (pTxTransferOp == NULL)
    {
      fprintf (stderr, "STAR: Unable to create the transfer operation to be transmitted\n");
      return 0;
    }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /***************************************************************/

  /* Submit the transmit operation */
  if (STAR_submitTransferOperation(txChannelId, pTxTransferOp) == 0)
    {
      fprintf (stderr, "STAR: ERROR occurred during transmit. \n\tTest failed.\n");
      return 0;
    }

  //Wait the operations to finish.
  STAR_TRANSFER_STATUS rxStatus, txStatus;

  /* Wait on the transmit operation completing */
  txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
						    op_timeout);
  if(txStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      fprintf (stderr, "STAR: ERROR occurred during transmit.\n\tTest failed.\n");
      return 0;
    }

  /* Submit the receive operation */
  if (STAR_submitTransferOperation(txChannelId, pRxTransferOp) == 0)
    {
      fprintf (stderr, "STAR: ERROR occurred during receive.\n\tTest failed.\n");
      return 0;
    }

  /* Wait on the receive operation completing */
  rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
						    op_timeout);

  if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      fprintf (stderr, "STAR: ERROR occurred during receive.\n\tTest failed.\n");
      return 0;
    }

    //Modify to parse the output.
    if (verbose_level == 1)
    {      
      printRxPackets((STAR_TRANSFER_OPERATION *)  pRxTransferOp);
    }

  /****************************************************************/
  /*    Process the command response                              */
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
              rply_data = RMAP_GetData( &packetStruct, &rply_data_length);
              
              if ( rply_data_length == 4)
              {
                unsigned int register_receiv_value;
                memcpy( &register_receiv_value, rply_data, 4);
                register_data_value =  reverse_4bytes_array(register_receiv_value);
                fprintf(stdout, "{\"0x%08x\" : 0x%08x}\n", address_to_read, register_data_value);
              }
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

