/*
  @file apus.c
  @author Juan Manuel Gómez
  @brief Spacewire Test Logical Routing Plato GR718B
  @details Configures the routing table to implement a logical routing.
  Enable the Spw Interfaces and configure the baudrate to run clk_div = 0.
  Configure the Routing table to 
  @param No parammeters needed.
  @example ./apus
  @copyright jmgomez CSIC-IAA
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

uint32_t GR718_ReadRegister(STAR_STREAM_ITEM **pTxStreamItem, uint32_t reg_addr);
uint32_t processRxOperation(STAR_TRANSFER_OPERATION * const pTransferOp);
uint32_t processPacket( STAR_SPACEWIRE_PACKET * streamItemPacket);
uint32_t processRegister(STAR_SPACEWIRE_PACKET * streamItemPacket);



//  pthread_attr_init(tinfo);
struct thread_info{
  pthread_t threadId;
  STAR_CHANNEL_ID channelId;
  char threadName[32];

  struct STAR_STREAM_ITEM **item;
  //For efficiency, it allows to access directly to the end.
  struct STAR_STREAM_ITEM *last;

};

void *tx_thread(void *arg)
{
  struct thread_info *params;
  STAR_TRANSFER_OPERATION *pTxTransferOp = NULL;
  uint32_t number_of_items;
  
  number_of_items = 1;

  params = (struct thread_info *) arg;


  /* Create the transmit transfer operation for the packet */
  pTxTransferOp = STAR_createTxOperation(params->item, number_of_items);
  if (pTxTransferOp == NULL)
    {
      puts("\nERROR: Unable to create the transfer operation to be transmitted");
      pthread_exit( 0);
    }
  else{
    printf("Tx operation created. \n");
  }

  //Dispose the stream item.
  if (params->item != NULL){
    free((*params->item));
  }
   
   
  /***************************************************************/
  /*    Submit the TX operations                                    */
  /*                                                             */
  /***************************************************************/
  /* Submit the transmit operation */
  if (STAR_submitTransferOperation(params->channelId, pTxTransferOp) == 0) {
    printf("\nERROR occurred during submit.  Test failed.\n");
    //	return 0;
  }

  //Wait the operations to finish.
  STAR_TRANSFER_STATUS  txStatus;
  txStatus = STAR_TRANSFER_STATUS_NOT_STARTED;
  while(txStatus != STAR_TRANSFER_STATUS_COMPLETE )
    {

      /* Wait on the transmit operation completing */
      txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
							STAR_INFINITE);

      if(txStatus != STAR_TRANSFER_STATUS_COMPLETE) 
 	{
	  printf("\nERROR Operation Time-Out.\n Retrying.\n"); 	  
	}
    }

  /* Dispose of the transfer operations */
  if (pTxTransferOp != NULL) 
    {
      STAR_disposeTransferOperation(pTxTransferOp);
    }
 
  //  pthread_exit(0);
  printf ("End of Thread TX.\n");
  return 0;
}

void *rx_thread(void *arg)
{
  struct thread_info params;
  STAR_STREAM_ITEM * pRxStreamItem;
  STAR_TRANSFER_OPERATION *pRxTransferOp = NULL;
  const uint32_t number_of_items = 1;
  uint32_t packetReceived = 0, i;
  STAR_TRANSFER_STATUS rxStatus;

  //It should be safer to reserve space for the params and do a copy. This avoid 
  //memory corruption in case the thread creator, frees the memory of the parammeters.
  params.channelId = ((struct thread_info *) arg)->channelId;

  // Create an RX operation to receive the Packet on port 2.
  pRxTransferOp = STAR_createRxOperation(number_of_items , STAR_RECEIVE_PACKETS);
  if (pRxTransferOp == NULL)
    {
      printf("[RXThread] : Error, unable to create receive operation.\n");
      pthread_exit(0);
    }

  /* Submit the receive operation */
  if (STAR_submitTransferOperation( params.channelId, pRxTransferOp) == 0)
    {
      printf("\nERROR occurred during receive.  Test failed.\n");
      pthread_exit(0);
    }

  printf ("RX opReady.\n");

  /* Wait on the receive operation completing */

  while(!packetReceived)
    {
      rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
							STAR_INFINITE);
      if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
	{
	  printf("\nERROR occurred during receive.  Test failed.\n");
	  return 0;
	}
      else
	{
	  printf("Packet Received.\n");
	  packetReceived = 1;	  
	}
    }

  //  processRxOperation( pRxTransferOp);
  packetReceived = STAR_getTransferItemCount(pRxTransferOp);
  if (packetReceived != 0 )
    {
      for(i=0; i< packetReceived; ++i)
	{
	  //Get element index i on the STREAM ITEM.
	  pRxStreamItem = STAR_getTransferItem (pRxTransferOp, i);
	  if (pRxStreamItem != NULL && pRxStreamItem->item != NULL &&
	      pRxStreamItem->itemType == STAR_STREAM_ITEM_TYPE_SPACEWIRE_PACKET)
	    {
	      processPacket( pRxStreamItem->item);
	    }
	}
    }

  if (pRxTransferOp != NULL)
    STAR_disposeTransferOperation(pRxTransferOp);

  //  pthread_exit(0);
  printf("End of thread RX.\n");
  
  return 0;
}



int __cdecl  main(int argc, char * argv[]){
  STAR_DEVICE_ID* devices;
  STAR_DEVICE_ID deviceId;
  unsigned int deviceCount;

  const uint32_t threads_nr = 0;
  const uint32_t max_threads_nr = 16;
  pthread_t threads[max_threads_nr];


  if (argc < 2)
    {
      printf ("Usage: ./%s reg_address.\n", argv[0]);
      printf ("reg_address: Address to read in hexadecimal, beginning with 0x.\n");
      return 0;
  }

  char address[] = {0x00, 0x00, 0x00, 0x00};    
  //Use "0x" to define the base of the number.
  if (strncmp(argv[1], "0x",2) != 0)
    {
      printf("Usage: ./%s address.\n", argv[1]);
      printf("The address should be in hexadecimal. 0xFFFFFFFF.");
      return 0;
    }
  uint32_t reg_address = strtoul(argv[1], NULL, 16);
  printf ("Proceed to read 0x%x : .\n", reg_address);
	    
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
  puts("\nChannel Spw 2 Opened and ready to communicate.  ");
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

  STAR_STREAM_ITEM **vTxStreamItem = NULL;
  unsigned int rxOp_itemCount= 0, txOp_itemCount = 0, txRTPOP_Itemcount= 0;
  U8 pData[4];
  unsigned long byteSize =  0;
 
  uint32_t number_of_items = 1;
  //Allocate Memory for TxStream Items
  vTxStreamItem = malloc(number_of_items* sizeof( STAR_STREAM_ITEM));
  if (!vTxStreamItem){
    puts("\nError: Could not allocate memory for the packet array.");    
    return 0;    
  }

  uint32_t status;
  status =  GR718_ReadRegister(vTxStreamItem, reg_address);
  if (status != 0){
    printf("Error generating the Stream.");
    return 0;
  }

  struct thread_info tx_params, tinfo;
  pthread_attr_t attr;
  uint32_t thread_status;

  //main should wait for threads before free resources, closing channel.
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  //Lets create a Treat to manage the RX operations of the TestPortchannel.
  tinfo.channelId = testPortChannel;
  strcpy(tinfo.threadName, "RxThread");
  
  tx_params.channelId = testPortChannel2;
  tx_params.item = vTxStreamItem;
  strcpy (tx_params.threadName, "TxThread");

  thread_status = pthread_create(& (tinfo.threadId), &attr, 
				  rx_thread, (void *) & tinfo);
  if (thread_status){
    printf("Error createing the RX thread.\n");
    return 0;
  }

  thread_status = pthread_create(& tx_params.threadId, &attr, 
				 tx_thread,(void *) &tx_params);
  if (thread_status)
    {
      printf("Error creating TX thread.\n");
    }

  printf ("Threads create.\n");

  void *st;
  pthread_join(tx_params.threadId, NULL);
  pthread_join(tinfo.threadId, NULL);

  printf("End of threads.\n\n");

  /* Close the channels */
  if (testPortChannel != 0U) {
    STAR_closeChannel(testPortChannel);
  }

  if (testPortChannel2 != 0U) {
    STAR_closeChannel(testPortChannel2);
  }

  //    pthread_exit(NULL);
    exit(0);

}

uint32_t processPacket( STAR_SPACEWIRE_PACKET * streamItemPacket){
  uint8_t *pStreamData = NULL;
  STAR_SPACEWIRE_ADDRESS *pStreamItemAddress = NULL;
  uint32_t streamDataSize = 0;
  uint32_t i;

  pStreamData = STAR_getPacketData( (STAR_SPACEWIRE_PACKET *) streamItemPacket,
				     &streamDataSize);	

  
  pStreamItemAddress = STAR_getPacketAddress ( (STAR_SPACEWIRE_PACKET *) streamItemPacket);
  /* if ( pStreamItemAddress != NULL){
    printf("Addres PATH: ");
    for(i=0; i<pStreamItemAddress->address->pathLen; ++i)
      printf( "\t0x%x", (*pStreamItemAddress->address->pPath + i) );
    printf("\n");
  }
*/
  //  if (
  for (i=0; i<streamDataSize; ++i)
    {
      printf( "\t0x%x" ,pStreamData[i]);
      if (!((i+1) % 8 ) )
	{
	  printf("\n");
	}
    }
  STAR_destroyAddress(pStreamItemAddress);
  STAR_destroyPacketData(pStreamData);
  
  return 0;
}


uint32_t processRegister(STAR_SPACEWIRE_PACKET * streamItemPacket){
  uint8_t *pStreamData = NULL;
  STAR_SPACEWIRE_ADDRESS * pStreamItemAddress = NULL;
  uint32_t streamDataSize = 0;
  
  uint32_t reg_value  = 0xA5A5A5A5;
  uint8_t *pReg_value = &reg_value;
  uint8_t *pRplyData;

  pStreamData = STAR_getPacketData( (STAR_SPACEWIRE_PACKET *) streamItemPacket, 
				    & streamDataSize);

  //  memcpy (& reg_value, pStreamData+(streamDataSize - (4 +1)), 4);
 
  //Bytes are receive in reverse order.
  pRplyData =  pStreamData + (streamDataSize - 1);
  uint32_t i = 0;
  while( i < 4){
    pRplyData --;
    pReg_value[i] = (*pRplyData);
    i ++;
  }
  printf ("The register value is: 0x%x \n", reg_value);

  return reg_value;

}


uint32_t processRxOperation(STAR_TRANSFER_OPERATION * const pTransferOp)
{
  uint32_t i, j, k;
  STAR_STREAM_ITEM *pRxStreamItem;
  
  uint32_t rxPacketCount;
  rxPacketCount = STAR_getTransferItemCount(pTransferOp);
  if (rxPacketCount != 0 )
    {
      for(i=0; i< rxPacketCount; ++i)
	{
	  //Get element index i on the STREAM ITEM.
	  pRxStreamItem = STAR_getTransferItem (pTransferOp, i);
	  if (pRxStreamItem != NULL && pRxStreamItem->item != NULL )
	    {
	      switch ( pRxStreamItem->itemType )
		{
		case STAR_STREAM_ITEM_TYPE_SPACEWIRE_PACKET:
		  printf("SpaceWire Packet Received.\n");
		  //processPacket((STAR_SPACEWIRE_PACKET *) pRxStreamItem->item);		  
		  processRegister( pRxStreamItem->item);
		  break;
		case STAR_STREAM_ITEM_TYPE_TIMECODE:
		  printf("Time Code Received.\n");
		  break;
		case STAR_STREAM_ITEM_TYPE_LINK_STATE_EVENT:
		  printf("Link State Event Received.\n");
		  break;
		case STAR_STREAM_ITEM_TYPE_DATA_CHUNK: 
		  printf("Spacewire Data Chunk Received.\n");
		  break;
		deafult:
		  printf("Unrecognized packet type.\n");
		}
	    }
	}
    }

  return rxPacketCount;
}

uint32_t GR718_ReadRegister(STAR_STREAM_ITEM **pTxStreamItem, uint32_t reg_addr)
{				       
  

  void *pFillPacket;
  unsigned long fillPacketLenCalculated, fillPacketLen;  
  
  U8 pTarget[] = {0,254};
  U8 pReply[] = {254};
  char status;  
  
  /* Calculate the length of a write command packet, with 4 bytes of data */
  fillPacketLenCalculated = RMAP_CalculateReadCommandPacketLength(2, 1,1);
  printf("The Read command packet will have a length of %ld bytes\n",
	 fillPacketLenCalculated);
  /* Allocate memory for the packet */
  pFillPacket = malloc(fillPacketLenCalculated);
  if (!pFillPacket)
    {
      puts("Couldn't allocate the memory for the command packet");
      return 1;
    }
  
  status = RMAP_FillReadCommandPacket(pTarget, 2, pReply, 1, 0, 0x00,
				      0, reg_addr, 0, 4, &fillPacketLen, NULL, 1, (U8 *)pFillPacket,
				       fillPacketLenCalculated);
  if (!status)
    {
      puts("Couldn't fill the write command packet");
      free(pFillPacket);
      return 1;
    }
  
  /* Create the packet to be transmitted */
  (*pTxStreamItem) = STAR_createPacket(NULL, (U8 *)pFillPacket, fillPacketLen,
				    STAR_EOP_TYPE_EOP);

  if ( (*pTxStreamItem) == NULL )
    {
      puts("\nERROR: Unable to create the packet to be transmitted");
      return 1;
    }

  return 0;
}
