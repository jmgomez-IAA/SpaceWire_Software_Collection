/*
  @file test_receiv.c
  @author Juan Manuel Gómez
  @brief Receive files forever and store it on a file.
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
#define _RX_INTERFACE 2
#define _PACKET_SIZE 64

#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4
#define _RX_BAUDRATE_MUL 2
#define _RX_BAUDRATE_DIV 4

#define _ADDRESS_PATH 2
#define _ADDRESS_PATH_SIZE 1

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
    puts("No device found\n");	
    return 0;
  } 

  /* We are working with the first device, what if I have more? */
  deviceId = devices[0];	
  if (deviceId == STAR_DEVICE_UNKNOWN){
    puts("Error dispositivo desconocido.\n");
    return 0;
  }

  /* Get hardware info*/
  CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
  CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);
  /* Display the hardware info*/
  printf("\nVersion: %s", versionStr);
  printf("\nBuildDate: %s", buildDateStr);	
  // Flashes the LEDS.
  if (CFG_MK2_identify(deviceId) == 0)
  {
    puts("\nERROR: Unable to identify device");
    return 0U;
  }

  /* Get the channels present on the device  */
  /* We need at least 2 Channels (0x00000007) */
  channelMask = STAR_getDeviceChannels(deviceId);
  if (channelMask != 7U)
  {
    puts("The device does not have channel 1 and 2 available.\n");
    return 0U;
  }

  int status_link = 0;
  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, txChannelNumber, clockRateParams);	
  if (status_link == 0){
    puts("Error setting the Tx link speed.\n");	
  }

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, rxChannelNumber, clockRateParams );
  if (status_link == 0){
    puts("Error setting the RX link speed.\n");	
  }

  /* Open channel 1 for Transmit*/
  txChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT,
					      txChannelNumber, TRUE);
  if (txChannelNumber == 0U)
  {
    puts("\nERROR: Unable to open TX channel 1\n");
    return 0;
  }
  /* Open channel 2 for Receipt*/
  rxChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_IN,
					      rxChannelNumber, TRUE);
  if (rxChannelId == 0U)
  {
    puts("\nERROR: Unable to open RX channel 2\n");
    return 0;
  }

  puts("Channels Opened.\n");	
	
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
  STAR_TRANSFER_STATUS rxStatus;
  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL, *pRxTransferOp = NULL;
  STAR_STREAM_ITEM *pTxStreamItem = NULL;
   
  /* Create the receive operations. */
  pRxTransferOp = STAR_createRxOperation(1, STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
  {
    puts("\nERROR: Unable to create receive operation");
    return 0;
  }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /***************************************************************/
  int counter= 0;
  while(counter < 20 || (rxStatus == STAR_TRANSFER_STATUS_COMPLETE))
  {
    /* Submit the receive operation */
    if (STAR_submitTransferOperation(txChannelId, pRxTransferOp) == 0)
    {
      printf("\nERROR occurred during receive.  Test Tfailed.\n");
      return 0;
    }

    /* Wait on the receive operation completing */
    rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
                                                      STAR_INFINITE);
    if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      printf("\nERROR occurred during receive.  Test failed.\n");
      return 0;
    }

    // Store the received packet in the file.
    /* Get the number of traffic items received */
    unsigned int rxPacketCount = STAR_getTransferItemCount(pRxTransferOp);
    if (rxPacketCount == 0)
    {
      printf("No packets received.\n");
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
          printf("\nERROR received an unexpected traffic type, or empty traffic item in item %lu\n", i);
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

          printf("Packet Number:\t%d\n", counter);
          int dat_iter = 0;
          for ( dat_iter= 0; dat_iter < streamDataSize; ++dat_iter){

            printf( "\t0x%x" ,pPacketBufferData[dat_iter]);
            if (! ((dat_iter+1) % 8) )
            {
              printf("\n");
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





