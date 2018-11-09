/*
  @file test_la_routing.c
  @author Juan Manuel Gómez
  @brief Spacewire Test Logical Routing Plato GR718B
  @details Configures the routing table to implement a logical routing.
  Enable the Spw Interfaces and configure the baudrate to run clk_div = 0.
  Configure the Routing table to 
  @param No parammeters needed.
  @example ./la_routing
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

#define VERSION_INFO "LA Route v1.0"

#include <errno.h>

#define STAR_INFINITE 30000

#define _SPW1_INTERFACE 1
#define _SPW2_INTERFACE 2
#define _PACKET_SIZE 64

#define _TX_BAUDRATE_MUL 2
#define _TX_BAUDRATE_DIV 4

#define _ADDRESS_PATH 2
#define _ADDRESS_PATH_SIZE 1

void freePacketArray(void **packetArray, int count);
uint32_t RTRCFG_WrRegToStream (STAR_STREAM_ITEM **pTxStreamItem, uint8_t *pValue, uint32_t reg_address);
unsigned long printRxPackets(STAR_TRANSFER_OPERATION * const pTransferOp);
void printPacket( STAR_SPACEWIRE_PACKET * StreamItemPacket);
uint32_t LoopBackPacketToStream (STAR_STREAM_ITEM **pTxStreamItem, uint8_t dst, uint8_t src, uint8_t *pValue, uint32_t reg_address);

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
  STAR_STREAM_ITEM **vTxStreamItem = NULL;
  unsigned int rxOp_itemCount= 0, txOp_itemCount = 0, txRTPOP_Itemcount= 0;

  unsigned long byteSize =  0;
  
  U8 pData[] = {0x00, 0x14, 0x02, 0x2E};

  uint32_t rtr_config_cmd = 12;
  uint32_t port_config_cmd = 10;
  txOp_itemCount = 9; // rtr_config_cmd + port_config_cmd;
  rxOp_itemCount = 1; // txOp_itemCount;
  
  // Packet is pTarget 2 address and pReply 1. Use alignment.
  uint32_t opCounter= 0;
  uint32_t reg_address = 0;
  uint32_t reg_base = 0x804;
  uint32_t reg_byteSize = 0x4;

  //Allocate Memory for TxStream Items
  vTxStreamItem = malloc(txOp_itemCount * sizeof( STAR_STREAM_ITEM));
  if (!vTxStreamItem){
    puts("\nError: Could not allocate memory for the packet array.");    
    return 0;    
  }

  uint32_t status;

  /**
   * Configures the Router 
   */
  //Packet 1: Enables IF1
  reg_address = 0x804;
  status = RTRCFG_WrRegToStream( vTxStreamItem, pData, reg_address);
  //Packet 2: Enables IF2
  reg_address = 0x808;
  status = RTRCFG_WrRegToStream( vTxStreamItem+1, pData, reg_address);
  //Packet 3: Enables IF10
  reg_address = 0x828;
  status = RTRCFG_WrRegToStream( vTxStreamItem+2, pData, reg_address);
  //Packet 4: Routing table 0x21 to IF2.
  reg_address = 0x84;
  U8 rtrICU_data[] = {0x00, 0x00, 0x00, 0x04};
  status = RTRCFG_WrRegToStream( vTxStreamItem+3, rtrICU_data, reg_address);
  //Packet 5: Routing table 0x21 Control.
  reg_address = 0x484;
  U8 rtr2ICU_data[] = {0x00, 0x00, 0x00, 0x0C};
  status = RTRCFG_WrRegToStream( vTxStreamItem+4, rtr2ICU_data, reg_address);
  //Packet 6: Routing table 0xFE to IF10
  reg_address = 0x3F8;
  U8 rtrDPU_data[] = {0x00, 0x00, 0x04, 0x00};
  status = RTRCFG_WrRegToStream( vTxStreamItem+5, rtrDPU_data, reg_address);
  //Packet 7: Routing table 0xFE control.
  reg_address = 0x7F8;
  U8 rtr2DPU_data[] = {0x00, 0x00, 0x00, 0x0C};
  status = RTRCFG_WrRegToStream( vTxStreamItem+6, rtr2DPU_data , reg_address);

  U8 val_data[] = {0x00, 0x12, 0x34, 0x56};
  // val_data = opCounter;
  status = LoopBackPacketToStream ( vTxStreamItem + 7, 0x21, 0xFE, val_data, reg_address);
  //val_data = opCounter + port_config_cmd;
  status = LoopBackPacketToStream ( vTxStreamItem + 8, 0xFE, 0xFE, val_data, reg_address);

  printf("Stream Ready.\n");

  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(vTxStreamItem, txOp_itemCount);
  if (pTxTransferOp == NULL)
    {
      puts("\nERROR: Unable to create the transfer operation to be transmitted");
      return 0;
    }
  else{
    printf("Tx operation created. \n");
  }

  // Create an RX operation to receive the Packet on port 2.
  
  /* Create the transfer operations. */
  pRxTransferOp = STAR_createRxOperation(rxOp_itemCount , STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
    {
      puts("\nERROR: Unable to create receive operation");
      return 0;
    }

  /* Submit the receive operation */
  if (STAR_submitTransferOperation(testPortChannel2, pRxTransferOp) == 0)
    {
      printf("\nERROR occurred during receive.  Test Tfailed.\n");
      return 0;
    }

  printf ("RX opReady.\n");

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


  /* Wait on the receive operation completing */
  rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
						    STAR_INFINITE);
  if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
    {
      printf("\nERROR occurred during receive.  Test failed.\n");
      return 0;
    }

  printRxPackets((STAR_TRANSFER_OPERATION *)  pRxTransferOp);
  //////////////////////////////////////////////////////////////////
  //Dispose the resources

  //Free allocated resources
  if (vTxStreamItem != NULL)
    free(vTxStreamItem);

  if (pRxTransferOp != NULL)
    STAR_disposeTransferOperation(pRxTransferOp);

  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL) {
    STAR_disposeTransferOperation(pTxTransferOp);
  }

  /* Close the channels */
  if (testPortChannel != 0U) {
    STAR_closeChannel(testPortChannel);
  }

  if (testPortChannel2 != 0U) {
    STAR_closeChannel(testPortChannel2);
  }


}


void freePacketArray(void **packetArray, int count){
  uint32_t opCounter = count;
  while(opCounter > 0){
    opCounter --;
    free(packetArray[opCounter]);
  }
  free(packetArray);
}


//insert a Write Register Operation to the TxStream
uint32_t LoopBackPacketToStream (STAR_STREAM_ITEM **pTxStreamItem, uint8_t dst, uint8_t src, uint8_t *pValue, uint32_t reg_address){ 

  void *pFillPacket;
  unsigned long fillPacketLenCalculated, fillPacketLen;

  int status_link;


  //Mandatory: We should include the address to Port 0 and the Target logical address 0xFE.
  U8 pTarget[] = {0xFE};

  //  pTarget[1] = dst;

  //Replies on the same port.
  U8 pReply[] = {0x21};
  //  pReply[1] = src;

  //Write operation of 4 bytes to update the value of a Register. 
  fillPacketLenCalculated = RMAP_CalculateWriteCommandPacketLength(1, 1, 4,1);  

  pFillPacket = malloc(fillPacketLenCalculated);
  if (!pFillPacket){
    puts("Error: Could not allocate mem for the packet.");
    return 1;
  }

  //FillWrPacket to reg_address
  status_link = RMAP_FillWriteCommandPacket(pTarget, 1, pReply, 1, 1, 0, 0, 0x00,
					    0, reg_address, 0, pValue, 4, 
					    &fillPacketLen, NULL, 1, (U8 *)pFillPacket,
					    fillPacketLenCalculated);
  if (!status_link ){
    puts ("\bError: Could not fill the packet. ");
    free(pFillPacket);
    return 2;
  }

  // Create the packet to be transmitted
  (*pTxStreamItem) = STAR_createPacket(NULL, (U8 *)pFillPacket, fillPacketLen,
					       STAR_EOP_TYPE_EOP);

  if (pTxStreamItem == NULL){
    puts("\nERROR: Unable to create the packet to be transmitted");
    free(pFillPacket);
    return 3;
  }

  free(pFillPacket);
  return 0;

}


 

//insert a Write Register Operation to the TxStream
uint32_t RTRCFG_WrRegToStream (STAR_STREAM_ITEM **pTxStreamItem, uint8_t *pValue, uint32_t reg_address){ 

  void *pFillPacket;
  unsigned long fillPacketLenCalculated, fillPacketLen;

  int status_link;


  //Mandatory: We should include the address to Port 0 and the Target logical address 0xFE.
  U8 pTarget[]= {0,254};

  //Replies on the same port.
  U8 pReply[] = {254};

  //Write operation of 4 bytes to update the value of a Register. 
  fillPacketLenCalculated = RMAP_CalculateWriteCommandPacketLength(2, 1, 4,1);  

  pFillPacket = malloc(fillPacketLenCalculated);
  if (!pFillPacket){
    puts("Error: Could not allocate mem for the packet.");
    return 1;
  }

  //FillWrPacket to reg_address
  status_link = RMAP_FillWriteCommandPacket(pTarget, 2, pReply, 1, 1, 0, 0, 0x00,
					    0, reg_address, 0, pValue, 4, 
					    &fillPacketLen, NULL, 1, (U8 *)pFillPacket,
					    fillPacketLenCalculated);
  if (!status_link ){
    puts ("\bError: Could not fill the packet. ");
    free(pFillPacket);
    return 2;
  }

  // Create the packet to be transmitted
  (*pTxStreamItem) = STAR_createPacket(NULL, (U8 *)pFillPacket, fillPacketLen,
					       STAR_EOP_TYPE_EOP);

  if (pTxStreamItem == NULL){
    puts("\nERROR: Unable to create the packet to be transmitted");
    free(pFillPacket);
    return 3;
  }

  free(pFillPacket);
  return 0;

}

void printPacket( STAR_SPACEWIRE_PACKET * StreamItemPacket){
    unsigned char* pTxStreamData = NULL;
    STAR_SPACEWIRE_ADDRESS *pStreamItemAddress = NULL;
    unsigned int pTxStreamDataSize = 0, i;
    pTxStreamData = STAR_getPacketData(
                    (STAR_SPACEWIRE_PACKET *)StreamItemPacket,
                    &pTxStreamDataSize);	

    pStreamItemAddress =  STAR_getPacketAddress ( (STAR_SPACEWIRE_PACKET *)StreamItemPacket);
/*
    printf("Packet  Address Path [Size: %d]\n", pStreamItemAddress->pathLength);
    printf("===================\n");
    for(i=0; i < pStreamItemAddress->pathLength ; ++i){
    	printf( "\t0x%x" ,pStreamItemAddress->pPath[i]);
	if (!((i+1) % 8 ) ){
	    printf("\n");
	}
    }
*/
	    printf("\n");

    printf("Packet  Data\n");
    printf("===================\n");
    for ( i= 0; i < pTxStreamDataSize; ++i){
    	printf( "\t0x%x" ,pTxStreamData[i]);
	if (!((i+1) % 8 ) ){
	    printf("\n");
	}
    }

    STAR_destroyAddress(pStreamItemAddress);
    STAR_destroyPacketData(pTxStreamData);
}


unsigned long printRxPackets(STAR_TRANSFER_OPERATION * const pTransferOp)
{
  unsigned long i;
  unsigned int rxPacketCount;
  STAR_STREAM_ITEM *pRxStreamItem;

  /* Get the number of traffic items received */
  rxPacketCount = STAR_getTransferItemCount(pTransferOp);
  if (rxPacketCount == 0)
    {
      printf("No packets received.\n");
    }
  else
    {
      /* For each traffic item received */
      for (i = 0U; i < rxPacketCount; i++)
        {
	  /* Get the packet */
	  pRxStreamItem = STAR_getTransferItem(pTransferOp, i);
	  if ((pRxStreamItem == NULL) || (pRxStreamItem->itemType !=
					  STAR_STREAM_ITEM_TYPE_SPACEWIRE_PACKET) ||
	      (pRxStreamItem->item == NULL))
            {
	      printf("\nERROR received an unexpected traffic type, or empty traffic item in item %lu\n",
		     i);
            }
	  else
            {
	      printf("\n\nPacket %d\n", i);
	      printPacket( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);
            }
        }
    }

  printf ("\n ***************  End of test ***************************\n");

  /* Return the error count */
  return rxPacketCount;
}
