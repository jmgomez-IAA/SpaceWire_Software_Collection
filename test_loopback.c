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
    unsigned long errorCount = 0U, i;
    unsigned int rxPacketCount;
    STAR_STREAM_ITEM *pRxStreamItem;
    char *pRxBuffer;
    U32 rxPacketLength;

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
		printPacket( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);
            }
        }
    }

    /* Return the error count */
    return rxPacketCount;
}



unsigned long comparePackets(STAR_TRANSFER_OPERATION * const pTransferOp,
    const unsigned long packetCount, const unsigned long packetSize,
    char * const pBuffer)
{
    unsigned long errorCount = 0U, i;
    unsigned int rxPacketCount;
    STAR_STREAM_ITEM *pRxStreamItem;
    char *pRxBuffer;
    U32 rxPacketLength;

    /* Get the number of traffic items received */
    rxPacketCount = STAR_getTransferItemCount(pTransferOp);
    if (rxPacketCount != packetCount)
    {
        printf("\nERROR expected to receive %lu packets but received %u.\n",
            packetCount, rxPacketCount);
        errorCount++;
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
                errorCount++;
            }
            else
            {
                pRxBuffer = (char *)STAR_getPacketData(
                    (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item,
                    &rxPacketLength);
                if ((pRxBuffer == NULL) || (rxPacketLength != packetSize))
                {
                    printf("\nERROR received a packet of length %u, expected length %lu in item %lu\n",
                        rxPacketLength, packetSize, i);
                    errorCount++;
                }
                else
                {
                    /* Compare the buffers and increment the error count if */
                    /* the buffers do not match */
                    errorCount += BufferCompareChar(pBuffer + (packetSize * i),
                        pRxBuffer, packetSize);
                }
                if (pRxBuffer != NULL)
                {
                    STAR_destroyPacketData((unsigned char *)pRxBuffer);
                }
            }
        }
    }

    /* Return the error count */
    return errorCount;
}



int __cdecl main(int argc, char *argv[]){

    STAR_DEVICE_ID* devices;
    STAR_DEVICE_ID deviceId;
    U32 deviceCount = 0U, i;
    unsigned int txChannelNumber, rxChannelNumber;
    STAR_CHANNEL_MASK channelMask;
    char *deviceName, s[256];
	 STAR_CFG_MK2_HARDWARE_INFO hardwareInfo;
    char versionStr[STAR_CFG_MK2_VERSION_STR_MAX_LEN];
    char buildDateStr[STAR_CFG_MK2_BUILD_DATE_STR_MAX_LEN];


    /* Get the list of devices of the specified type */
    devices = STAR_getDeviceListForType(STAR_DEVICE_TXRX_SUPPORTED , &deviceCount);
    if (devices == NULL){
	puts("No device found\n");	
	return 0;
    } 

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
    }

	//Configure Channel 1 for TX
    /* Get the channels present on the device */
    channelMask = STAR_getDeviceChannels(deviceId);
    if (channelMask != 7U)
    {
        puts("La mascara de canales indica que no hay suficientes canales.\n");
        return 0U;
    }


    txChannelNumber = 1;
    rxChannelNumber = 2;
    STAR_CHANNEL_ID rxChannelId = 0U, txChannelId = 0U;

    STAR_CFG_MK2_BASE_TRANSMIT_CLOCK clockRateParams;

  /* Set port 2 to have a base transmit rate of 150Mbps
        ie (100*3/4)*2    */
    clockRateParams.multiplier = 2;
    clockRateParams.divisor = 4;

	int status_link = 0;
//	status_link = CFG_MK2_setBaseTransmitClock(deviceId, txChannelNumber, clockRateParams);
//	status_link = CFG_BRICK_MK3_setTransmitClock(deviceId, txChannelNumber, clockRateParams );
	status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, txChannelNumber, clockRateParams );
	
	if (status_link == 0){
		puts("Error setting the Tx link speed.\n");	
	}

	status_link = CFG_BRICK_MK3_setBaseTransmitClock(deviceId, rxChannelNumber, clockRateParams );
	if (status_link == 0){
		puts("Error setting the RX link speed.\n");	
	}

   /* Open channel 1 for Transmit*/
    txChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_OUT,
        txChannelNumber, TRUE);
    if (txChannelNumber == 0U)
    {
        puts("\nERROR: Unable to open TX channel 1\n");
        return 0;
    }
	
    rxChannelId = STAR_openChannelToLocalDevice(deviceId, STAR_CHANNEL_DIRECTION_IN,
        rxChannelNumber, TRUE);
    if (rxChannelId == 0U)
    {
        puts("\nERROR: Unable to open RX channel 2\n");
        return 0;
    }


	puts("Channels Opened.\n");	
	
    unsigned long byteSize = 2048;
    char *pTxBuffer = NULL;
    STAR_TRANSFER_OPERATION *pTxTransferOp = NULL, *pRxTransferOp = NULL;
    STAR_STREAM_ITEM *pTxStreamItem = NULL;

/* Allocate memory for the transmit buffer */
    pTxBuffer = (char *)calloc(1U, byteSize);
    if (pTxBuffer == NULL)
    {
        puts("\nERROR: Unable to allocate memory for transmit buffer");
        return 0;
    }

    for(i=0; i< byteSize; ++i){
	pTxBuffer[i] = i;
    }

/* Create a SpaceWire address from the path */

    unsigned char newPath[128];
    unsigned int pathLen = 0;
    STAR_SPACEWIRE_ADDRESS *pAddressPath = NULL;

    newPath[0] = 2;
    newPath[1] = 2;
    pathLen = 1;
    pAddressPath = STAR_createAddress(newPath, pathLen);

// Create the transfer operations.

 pRxTransferOp = STAR_createRxOperation(1, STAR_RECEIVE_PACKETS);
    if (pRxTransferOp == NULL)
    {
        puts("\nERROR: Unable to create receive operation");
	return 0;
    }

    /* Create the packet to be transmitted */
    pTxStreamItem = STAR_createPacket(pAddressPath, (U8 *)pTxBuffer, byteSize,
        STAR_EOP_TYPE_EOP);
    if (pTxStreamItem == NULL)
    {
        puts("\nERROR: Unable to create the packet to be transmitted");
	return 0;
    }


    printPacket((STAR_SPACEWIRE_PACKET *)pTxStreamItem->item);

    /* Create the transmit transfer operation for the packet */
    pTxTransferOp = STAR_createTxOperation(&pTxStreamItem, 1U);
    if (pTxTransferOp == NULL)
    {
        puts("\nERROR: Unable to create the transfer operation to be transmitted");
	return 0;
    }

//Submit the operations
unsigned long  testNum = 0;
      /* Submit the receive operation */
        if (STAR_submitTransferOperation(rxChannelId, pRxTransferOp) == 0)
        {
            printf("\nERROR occurred during receive.  Test %lu failed.\n",
                testNum + 1U);
		return 0;
        }

        /* Submit the transmit operation */
        if (STAR_submitTransferOperation(txChannelId, pTxTransferOp) == 0)
        {
            printf("\nERROR occurred during transmit.  Test %lu failed.\n",
                testNum + 1U);
		return 0;
        }

//Wait the operations to finish.
STAR_TRANSFER_STATUS rxStatus, txStatus;

        /* Wait on the transmit operation completing */
        txStatus = STAR_waitOnTransferOperationCompletion(pTxTransferOp,
            STAR_INFINITE);
        if(txStatus != STAR_TRANSFER_STATUS_COMPLETE)
        {
            printf("\nERROR occurred during transmit.  Test %lu failed.\n",
                testNum + 1U);
		return 0;
        }

        /* Wait on the receive operation completing */
        rxStatus = STAR_waitOnTransferOperationCompletion(pRxTransferOp,
            STAR_INFINITE);
        if (rxStatus != STAR_TRANSFER_STATUS_COMPLETE)
        {
            printf("\nERROR occurred during receive.  Test %lu failed.\n",
                testNum + 1U);
		return 0;
        }


//Compare the results
	int errorCount;
       errorCount += comparePackets(pRxTransferOp, 1U, byteSize, pTxBuffer);

	printf("El numero de errores es: %i.\n", errorCount);


    printRxPackets((STAR_TRANSFER_OPERATION *)  pRxTransferOp);

    /* Dispose of the transfer operations */
    if (pTxTransferOp != NULL)
    {
        STAR_disposeTransferOperation(pTxTransferOp);
    }
    if (pRxTransferOp != NULL)
    {
        STAR_disposeTransferOperation(pRxTransferOp);
    }

    /* Destroy the packet transmitted */
    if (pTxStreamItem != NULL)
    {
        STAR_destroyStreamItem(pTxStreamItem);
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
}





