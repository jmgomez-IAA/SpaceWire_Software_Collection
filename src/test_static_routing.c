/*
  @file test_static_routing.c
  @author Juan Manuel Gómez
  @brief Test Logical Routing Plato GR718B and GR712RC
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

uint32_t RTRCFG_WrRegToStream (STAR_STREAM_ITEM **pTxStreamItem, uint8_t *pValue, uint32_t reg_address);

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
  STAR_CHANNEL_ID testPortChannel;
  STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  int status_link = 0;
  clockRateParams.multiplier = _TX_BAUDRATE_MUL;
  clockRateParams.divisor = _TX_BAUDRATE_DIV;

  status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, _SPW1_INTERFACE, clockRateParams);
  if ( status_link == 0){
    puts("\nError: Could not configure baudrate.");
    return 0;
  }

  testPortChannel = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_INOUT, _SPW1_INTERFACE, TRUE);
  if(testPortChannel == 0){
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
  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL;
  STAR_STREAM_ITEM **vTxStreamItem = NULL;
  unsigned int txOp_itemCount = 0;

  unsigned long byteSize =  0;
  
  U8 pData[] = {0x00, 0x14, 0x02, 0x2E};

  uint32_t rtr_config_cmd = 12;
  uint32_t port_config_cmd = 10;
  txOp_itemCount = 0xA; // rtr_config_cmd + port_config_cmd;
  
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
  //Packet 3: Enables IF3
  reg_address = 0x80C;
  status = RTRCFG_WrRegToStream( vTxStreamItem+2, pData, reg_address);
  //Packet 4: Enables IF4
  reg_address = 0x810;
  status = RTRCFG_WrRegToStream( vTxStreamItem+3, pData, reg_address);
  //Packet 5: Enables IF5
  reg_address = 0x814;
  status = RTRCFG_WrRegToStream( vTxStreamItem+4, pData, reg_address);
  //Packet 6: Enables IF6
  reg_address = 0x818;
  status = RTRCFG_WrRegToStream( vTxStreamItem+5, pData, reg_address);
  //Packet 7: Enables IF7
  reg_address = 0x81C;
  status = RTRCFG_WrRegToStream( vTxStreamItem+6, pData, reg_address);
  //Packet 8: Enables IF8
  reg_address = 0x820;
  status = RTRCFG_WrRegToStream( vTxStreamItem+7, pData, reg_address);

  //Packet 9: Routing table 0x21 (33) to IF1.
  reg_address = 0x84;
  U8 rtrICU_data[] = {0x00, 0x00, 0x00, 0x02};
  status = RTRCFG_WrRegToStream( vTxStreamItem+8, rtrICU_data, reg_address);
  //Packet A: Routing table 0x21 Control.
  reg_address = 0x484;
  U8 rtr2ICU_data[] = {0x00, 0x00, 0x00, 0x0C};
  status = RTRCFG_WrRegToStream( vTxStreamItem+9, rtr2ICU_data, reg_address);

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

  //Dispose the resources

  //Free allocated resources
  if (vTxStreamItem != NULL)
    free(vTxStreamItem);

  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL) {
    STAR_disposeTransferOperation(pTxTransferOp);
  }

  /* Close the channels */
  if (testPortChannel != 0U) {
    STAR_closeChannel(testPortChannel);
  }

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

