/*
  @file stipa.c
  @author Juan Manuel Gómez
  @brief Spacewire Test Interface Plato Audit GR718 
  @details Audit the values of the GR718B to monitor the 
  status of the system.
  @param No parammeters needed.
  @example ./stipa  
  @copyright jmgomez CSIC-IAA
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "system_config.h"
#include "utility.h"
#include "star-dundee_types.h"
#include "star-api.h"
//#include "cfg_api_router.h"
#include "cfg_api_mk2.h"
#include "cfg_api_mk2_types.h"
//#include "cfg_api_brick_mk3.h"
#include "rmap_packet_library.h"

#define VERSION_INFO "Stipa v1.0"

#include <errno.h>

#define STAR_INFINITE 30000

#define _SPW1_INTERFACE 1
#define _SPW2_INTERFACE 2
#define _PACKET_SIZE 64

#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4

#define _ADDRESS_PATH 2
#define _ADDRESS_PATH_SIZE 1

void freePacketArray(void **packetArray, int count){
  uint32_t opCounter = count;
      while(opCounter > 0){
	opCounter --;
	free(packetArray[opCounter]);
      }
      free(packetArray);
    }

int __cdecl  main(int argc, char * argv[]){
  STAR_DEVICE_ID* devices;
  STAR_DEVICE_ID deviceId;
  unsigned int deviceCount;

  //Initialize
  devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED, & deviceCount);
  if (devices == NULL){
    puts("Error: No compatible device found.\n");
    return 0;
  }

  deviceId = devices[0];
  if (deviceId == STAR_DEVICE_UNKNOWN){
    puts("Error: Unknown device.\n");
    return 0;
  }

  STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
  char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
  char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];

  CFG_MK2_getHardwareInfo(deviceId, &hardwareInfo);
  CFG_MK2_hardwareInfoToString(hardwareInfo, versionStr, buildDateStr);

  printf("\n Version: %s", versionStr);
  printf("\n Build Date: %s", buildDateStr);

  if (CFG_MK2_identify(deviceId) == 0){
    puts("\nError: Unable to indentify device.");
  }

  STAR_CHANNEL_MASK channelMask;
  channelMask = STAR_getDeviceChannels(deviceId);
  if ( channelMask != 7U){
    puts("\nError: Almost one interface is required.");
    return 0;
  }

  /***************************************************************/
  /*        Configurate Baudrate                                 */
  /*                                                             */
  /* Channel 1 = Transmit interface                              */
  /* Channel 2 = Receive  interface                              */
  /* BaudRate  = 100 Mbps (100*2/4)*2                            */
  /* Packet Size = 1 KB                                          */
  /***************************************************************/
  STAR_CHANNEL_ID testPortChannel, testPortChannel2;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  int status_link = 0;
  clockRateParams.multiplier = _TX_BAUDRATE_MUL;
  clockRateParams.divisor = _TX_BAUDRATE_DIV;

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, _SPW1_INTERFACE, clockRateParams);
  if ( status_link == 0){
    puts("\nError: Could not configure baudrate.");
    return 0;
  }

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, _SPW2_INTERFACE, clockRateParams);
  if ( status_link == 0){
    puts("\nError: Could not configure baudrate.");
    return 0;
  }
  
  testPortChannel = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT, _SPW1_INTERFACE, TRUE);
  if(testPortChannel == 0){
    puts("\nError : Unable to open the Channel.");
  }

  testPortChannel2 = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT, _SPW2_INTERFACE, TRUE);
  if(testPortChannel2 == 0){
    puts("\nError : Unable to open the Channel.");
  } 
 
  puts("\nChannel Spw 1 Opened and ready to communicate.  ");
  puts("\n************************************************\n");

  /*****************************************************************/
  /*    Allocate memory for the transmit buffer and construct      */
  /*    the packet to transmit.                                    */
  /*****************************************************************/
  /*                                                               */
  /* We should read the regiters of the GR718B. We should transmit */
  /* RMAP Read Commands to GR718 Port 0.                           */
  /* The Read Commands data size is always 1 register each time,   */
  /* that is 4 byte per command.                                   */
  /*****************************************************************/
  /*       Create the Transmit and Receive Operations              */
  STAR_TRANSFER_OPERATION **pConfigRouterTransferOp = NULL;
  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL, *pRxTransferOp = NULL;
  STAR_STREAM_ITEM **pTxStreamItem = NULL;
  unsigned int rxOp_itemCount= 0, txOp_itemCount = 0;

  unsigned long byteSize =  0;
  
  void **pFillPacket;
  unsigned long fillPacketLen, fillPacketLenCalculated;

  U8 pTarget[]= {0,254};
  U8 pReply[] = {254};
  U8 pData[] = {0x00, 0x14, 0x02, 0x2E};

  txOp_itemCount = 10;
  
  // Packet is pTarget 2 address and pReply 1. Use alignment.
  uint32_t opCounter= 0;
  uint32_t reg_address = 0;
  uint32_t reg_base = 0x880;
  uint32_t reg_byteSize = 0x4;

  //Allocate Memmory for packet Matrix
  pFillPacket = (void **) malloc(txOp_itemCount*sizeof(void *));
  if (!pFillPacket){
    puts("\nError: Could not allocate memory for the packet array.");
    return 0;
  }

  pTxStreamItem = malloc(txOp_itemCount * sizeof( STAR_STREAM_ITEM));
  if (!pTxStreamItem){
    puts("\nError: Could not allocate memory for the packet array.");
    freePacketArray(pFillPacket, 0);
    return 0;    
  }

  fillPacketLenCalculated = RMAP_CalculateWriteCommandPacketLength(2, 1, 4,1);
  for(opCounter= 0; opCounter < txOp_itemCount; ++ opCounter){
    //malloc Packet opCounter.
    pFillPacket[opCounter] = malloc(fillPacketLenCalculated);
    if (!pFillPacket[opCounter]){
      puts("Error: Could not allocate mem for the packet.");
      freePacketArray(pFillPacket, opCounter);
      return 0;
    }

    printf("Packet %d ready to Fill.\n", opCounter);

    //FillWrPacket to reg_address
    reg_address = reg_base + reg_byteSize*opCounter;
    status_link = RMAP_FillWriteCommandPacket(pTarget, 2, pReply, 1, 1, 0, 0, 0x00,
					 0, reg_address, 0, pData, 4, 
					 &fillPacketLen, NULL, 1, (U8 *)pFillPacket[opCounter],
					 fillPacketLenCalculated);
    if (!status_link ){
      puts ("\bError: Could not fill the packet. ");
      freePacketArray (pFillPacket, opCounter);
      return 0;
    }
 
    printf("Filled Wr packet %d to addr 0x%x .\n", opCounter, reg_address);

  /* Create the packet to be transmitted */
    pTxStreamItem[opCounter] = STAR_createPacket(NULL, (U8 *)pFillPacket[opCounter], fillPacketLen,
				    STAR_EOP_TYPE_EOP);

    if (pTxStreamItem[opCounter] == NULL){
      puts("\nERROR: Unable to create the packet to be transmitted");
      freePacketArray(pFillPacket, txOp_itemCount);
      return 0;
    }

  }

  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(pTxStreamItem, txOp_itemCount);
  if (pTxTransferOp == NULL)
    {
      puts("\nERROR: Unable to create the transfer operation to be transmitted");
      return 0;
    }

  rxOp_itemCount = 1;
  /* Create the receive operations. */
  pRxTransferOp = STAR_createRxOperation(rxOp_itemCount, STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
    {
      puts("\nERROR: Unable to create receive operation");
      return 0;
    }

  /***************************************************************/
  /*    Submit the operations                                    */
  /*                                                             */
  /***************************************************************/

  /* Submit the transmit operation */
  if (STAR_submitTransferOperation(testPortChannel, pTxTransferOp) == 0) {
    printf("\nERROR occurred during transmit.  Test failed.\n");
    return 0;
  }

  //Wait the operations to finish.
  STAR_TRANSFER_STATUS rxStatus, txStatus;

  /* Wait on the transmit operation completing */
  txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
						    STAR_INFINITE);
  if(txStatus != STAR_TRANSFER_STATUS_COMPLETE) {
    printf("\nERROR occurred during transmit.  Test failed.\n");
    return 0;
  }

  //Free allocated resources
  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL) {
    STAR_disposeTransferOperation(pTxTransferOp);
  }

  if (pRxTransferOp != NULL) {
    STAR_disposeTransferOperation(pRxTransferOp);
  }

  if (pFillPacket != NULL){
    //    RMAP_FreeBuffer(pFillPacket);
    freePacketArray(pFillPacket, txOp_itemCount);
  }

  /* Close the channels */
  if (testPortChannel != 0U) {
    STAR_closeChannel(testPortChannel);
  }


}
