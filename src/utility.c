/**
 * \file utility.c
 *
 * \brief General utility functions for the STAR-System Test program.
 *
 * \author STAR-Dundee Ltd.\n
 *         STAR House\n
 *         166 Nethergate\n
 *         Dundee, DD1 4EE\n
 *         Scotland, UK\n
 *         e-mail: support@star-dundee.com
 *
 * Contains utility functions used by the STAR-System Test program.
 *
 * Copyright &copy; 2013 STAR-Dundee Ltd
 */


#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "utility.h"



unsigned int reverse_4bytes_array(unsigned int lsb_integer)
{
    unsigned int tmp = 0;
    
    tmp = ((lsb_integer & 0x000000FF) << 24) | ((lsb_integer & 0x0000FF00) << 8) | ((lsb_integer & 0x00FF0000) >> 8) | lsb_integer >> 24;
    return (tmp);
}

/**
* Process the receive message and outputs
*/

/* Display the type of a packet. */
void display_packet_type(RMAP_PACKET_TYPE type)
{
    switch (type)
    {
        case RMAP_WRITE_COMMAND:
            puts("The packet is an RMAP write command.");
            break;
        
        case RMAP_WRITE_REPLY:
            puts("The packet is an RMAP write reply.");
            break;
        
        case RMAP_READ_COMMAND:
            puts("The packet is an RMAP read command.");
            break;
        
        case RMAP_READ_REPLY:
            puts("The packet is an RMAP read reply.");
            break;
        
        case RMAP_READ_MODIFY_WRITE_COMMAND:
            puts("The packet is an RMAP read/modify/write command.");
            break;
        
        case RMAP_READ_MODIFY_WRITE_REPLY:
            puts("The packet is an RMAP read/modify/write reply.");
            break;
        
        case RMAP_INVALID_PACKET_TYPE:
            puts("The packet is of an invalid RMAP packet type.");
            break;
            
        default:
            puts("The packet is of an unexpected type.");
    }
}

/* Display the status of a packet. */
void display_packet_status(RMAP_STATUS status)
{
    switch (status)
    {
        case RMAP_SUCCESS:
            puts("The status indicates the command completed successfully.");
            break;
        case RMAP_GENERAL_ERROR:
            puts("The status indicates a general error that does not fit into the other error cases.");
            break;
        case RMAP_UNUSED_PACKET_TYPE_OR_COMMAND_CODE:
            puts("The status indicates the packet type or command is an unexpected value.");
            break;
        case RMAP_INVALID_KEY:
            puts("The status indicates the key did not match that expected by the target user application.");
            break;
        case RMAP_INVALID_DATA_CRC:
            puts("The status indicates an invalid data CRC.");
            break;
        case RMAP_EARLY_EOP:
            puts("The status indicates an EOP was detected before the end of the data.");
            break;
        case RMAP_TOO_MUCH_DATA:
            puts("The status indicates there was more data than was expected.");
            break;
        case RMAP_EEP:
            puts("The status indicates an EEP was  encountered in the packet after the header.");
            break;
        case RMAP_VERIFY_BUFFER_OVERRUN:
            puts("The status indicates verify before write was enabled in the command but not enough buffer space was available to receive the full command.");
            break;
        case RMAP_COMMAND_NOT_IMPLEMENTED_OR_AUTHORISED:
            puts("The status indicates the target user application did not authorise the requested operation.");
            break;
        case RMAP_RMW_DATA_LENGTH_ERROR:
            puts("The status indicates the amount of data in a read/modify/write command is invalid.");
            break;
        case RMAP_INVALID_TARGET_LOGICAL_ADDRESS:
            puts("The status indicates the target logical address was not the value expected by the target.");
            break;
        default:
            puts("The status is an unexpected value.");
    }
}

void dumpPacket( STAR_SPACEWIRE_PACKET * StreamItemPacket){
    unsigned char* pTxStreamData = NULL;
    STAR_SPACEWIRE_ADDRESS *pStreamItemAddress = NULL;
    unsigned int pTxStreamDataSize = 0, i;
    pTxStreamData = STAR_getPacketData(
                    (STAR_SPACEWIRE_PACKET *)StreamItemPacket,
                    &pTxStreamDataSize);    

    pStreamItemAddress =  STAR_getPacketAddress ( (STAR_SPACEWIRE_PACKET *)StreamItemPacket);
        printf("\n");

    printf("Packet  Data\n");
    printf("===================\n");
    for ( i= 0; i < pTxStreamDataSize; ++i){
        printf( "\t0x%x" , pTxStreamData[i]);
    if (!((i+1) % 8 ) ){
        printf("\n");
    }
    }

    STAR_destroyAddress(pStreamItemAddress);
    STAR_destroyPacketData(pTxStreamData);
}


unsigned long dumpTransferOperation(STAR_TRANSFER_OPERATION * const pTransferOp)
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
            
                dumpPacket( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);
            }
        }
    }

  /* Return the error count */
  return rxPacketCount;
}



void printReply(STAR_SPACEWIRE_PACKET * StreamItemPacket)
{

    rmap_reply_header_t rply_header;
    rmap_reply_data_t rply_data;
    unsigned char* pTxStreamData = NULL;
    STAR_SPACEWIRE_ADDRESS *pStreamItemAddress = NULL;
    unsigned int pTxStreamDataSize = 0, i;

    // Get the Data of the packet
    pTxStreamData = STAR_getPacketData(
                    (STAR_SPACEWIRE_PACKET *)StreamItemPacket,
                    &pTxStreamDataSize);  

    pStreamItemAddress =  STAR_getPacketAddress ( (STAR_SPACEWIRE_PACKET *)StreamItemPacket);

    rply_header.init_address        = pTxStreamData[0];
    rply_header.protocol_id         = pTxStreamData[1];           //  Byte 2
    rply_header.instruction_field   = pTxStreamData[2];     //  Byte 3
    rply_header.cmd_status          = pTxStreamData[3];            //  Byte 4
    rply_header.target_addr         = pTxStreamData[4];           //  Byte 5
    rply_header.tans_ident          = pTxStreamData[5]<<8 | pTxStreamData[6];           //  Byte 6-7
    rply_header.reserved_1          = pTxStreamData[7];           //  Byte 8
    rply_header.data_length         = pTxStreamData[8]<<16 | pTxStreamData[9]<<8 | pTxStreamData[10];           //  Byte 9,10,11
    rply_header.header_crc          = pTxStreamData[11];             //  Byte 12

    rply_data.data_length = rply_header.data_length;
    rply_data.data_chunk = malloc (rply_data.data_length );
    if (rply_data.data_chunk != NULL)
    {
      memcpy(rply_data.data_chunk, pTxStreamData+RPLY_PACKET_DATA_OFFSET, rply_data.data_length);

      printf("\n");
      printf(" RAMP REPLY PACKET \n");
      printf("===================\n");
      printf("\nFirst byte transmitted.\nReply SpW Address\n .... ");
      printf("\nByte\t| Description\t\t\t| Value");
      printf("\n0\t| Reply SpW Address\t\t| ");
      printf("\n1\t| Initiator Logical Address\t| 0x%02X", rply_header.init_address);
      printf("\n2\t| Protocol Identifier\t\t| 0x%02X", rply_header.protocol_id);
      printf("\n3\t| Instruction\t\t\t| 0x%02X" , rply_header.instruction_field);
      printf("\n4\t| Status\t\t\t| 0x%02X", rply_header.cmd_status);
      printf("\n5\t| Target Logical Address\t| 0x%02X", rply_header.target_addr);
      printf("\n6-7\t| Transaction Identifier \t| 0x%04X", rply_header.tans_ident);
      printf("\n8\t| Reserved = 0\t\t\t| 0x%02X", rply_header.reserved_1);
      printf("\n9-11\t| Data Length\t\t\t| 0x%06X", rply_header.data_length);
      printf("\n12\t| Header CRC\t\t\t| 0x%02X", rply_header.header_crc);

      printf("\n Data\n");

      for( i = 0; i < rply_data.data_length; ++i)
      {
        rply_data.data_chunk[i];
        printf( "\t0x%02X" , rply_data.data_chunk[i]);

        if (!((i+1) % 8 ) )
        {
          printf("\n");
        }        
      }

      printf("\n..................\n");
    }
    
    free(rply_data.data_chunk);

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
          printReply( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);
        //dumpPacket( (STAR_SPACEWIRE_PACKET *)pRxStreamItem->item);
            }
        }
    }

  /* Return the error count */
  return rxPacketCount;
}


/******************************************************************************/
/*                                                                            */
/* Generates a 16-bit random number                                           */
/*                                                                            */
/******************************************************************************/
unsigned int random16(void)
{
    unsigned int value = rand();

    if ((unsigned int )rand() > (RAND_MAX / 2))
    {
        value += RAND_MAX;
    }

    return value;
}


/******************************************************************************/
/*                                                                            */
/* Generates a 32-bit random number                                           */
/*                                                                            */
/******************************************************************************/
unsigned int random32(void)
{
    unsigned int value;

    value = random16();
    value *= 0x10000U;
    value += random16();

    return value;
}



/******************************************************************************/
/*                                                                            */
/* Plain format-less File read/write                                          */
/* Reads a file into a buffer                                                 */
/*                                                                            */
/******************************************************************************/
int file_read(const char fname[], char * const pBuffer,
    unsigned int * const byteSize)
{
    FILE *infile;
    unsigned int count;

    *byteSize = 0U;
    infile = fopen(fname, "rb");
    if (infile == NULL)
    {
        printf("file_read:Trouble opening file\n");
        return 0;
    }

    /* Cycle until end of file reached: */
    while (feof(infile) == 0)
    {
        /* Attempt to read in a chunk of bytes */
        count = fread((pBuffer + *byteSize), sizeof(char), 100U, infile);
        if (ferror(infile) != 0)
        {
            printf("Read error in file_read: After reading %u\n", *byteSize);
            fclose(infile);
            return 0;
        }
        *byteSize += count;

    }

    fclose(infile);
    return 1;
}



/******************************************************************************/
/*                                                                            */
/* Plain format-less File read/write                                          */
/* Reads PART of a file into a buffer                                         */
/*                                                                            */
/******************************************************************************/
int file_read_chunk(const char fname[], char * const pBuffer, const long offset,
    const unsigned int size)
{
    FILE *infile;
    unsigned int count;

    infile = fopen(fname, "rb");
    if (infile == NULL)
    {
        printf("file_read:Trouble opening file\n");
        return 0;
    }

    /* Move file steam pointer to offset */
    fseek(infile,offset,SEEK_SET);

    /* Attempt to read in a chunk of bytes */
    count = (unsigned int)fread(pBuffer, sizeof(char), size, infile);

    if ((ferror(infile) != 0) || (count < size))
    {
        printf("ERROR trying to read file %s at byte point %ld.\n", fname, offset);
        fclose(infile);
        return 0;
    }

    fclose(infile);
    return 1;
}


/******************************************************************************/
/*                                                                            */
/*  Receives a fixed size message.                                            */
/*  Returns FALSE if data could not be read.                                  */
/*                                                                            */
/******************************************************************************/
int file_append_chunk(const unsigned char * const pData, const long dataSize,
    const char fname[])
{
    FILE *outfile;
    int returnValue = 1;

    /* Open the file to write to */
    outfile = fopen(fname, "ab");
    if (outfile == NULL)
    {
        printf("\nfile_append_chunk: Trouble opening file: %s\n", fname);
        return 0;
    }

    fwrite(pData, 1U, dataSize, outfile);

    if (ferror(outfile) != 0)
    {
        printf("\nWrite error in file_append_chunk.\n");
        returnValue = 0;
    }

    fclose(outfile);

    return returnValue;
}



/******************************************************************************/
/*                                                                            */
/*  FillBuffer                                                                */
/*  Fills a buffer with different data types. (zeros, ones, random, count)    */
/*                                                                            */
/******************************************************************************/
void FillBufferRandomChar(char * const pBuffer, const unsigned int size,
    const int dataType)
{
    unsigned int n;
    switch (dataType)
    {
    case DATA_TYPE_0:       /* Fill with zeros */
        memset(pBuffer, 0, size);
        break;

    case DATA_TYPE_1:       /* Fill with ones */
        memset(pBuffer, 1, size);
        break;

    case DATA_TYPE_RANDOM:      /* Fill with random values */
        for (n = 0U; n < size; n++)
        {
            *(pBuffer + n) = (char) random32();
        }
        break;

    case DATA_TYPE_COUNT:       /* Fill with simple count */
        for (n = 0U; n < size; n++)
        {
            *(pBuffer + n) = (char)n;
        }
        break;

    case DATA_TYPE_NOT_COUNT:   /* Fill with not of simple count */
        for (n = 0U; n < size; n++)
        {
            *(pBuffer + n) = 0xFF - (char)n;
        }
        break;
    }
}



/******************************************************************************/
/*                                                                            */
/*  Buffer compare function: Used instead of memcmp() for debugging           */
/*  Returns the number of errors found                                        */
/*                                                                            */
/******************************************************************************/
unsigned long BufferCompareChar(const char * const pBuffer1,
    const char * const pBuffer2, const unsigned long size)
{
    unsigned int errorCount = 0U;
    unsigned long i;

    for (i = 0U; i < size; i++)
    {
        if (*(pBuffer1 + i) != *(pBuffer2 + i))
        {
            errorCount++;
            printf("Error in byte %8lu, should be 0x%2x actually 0x%2x\n", i,
                *(pBuffer1 + i), *(pBuffer2 + i));
        }
    }

    return errorCount;
}



/**
 * Copy a numerical value into a buffer, making sure that the MSB is in the
 * first byte of the buffer.
 *
 * @param pBuffer the buffer to copy the number in to
 * @param number the number to copy in to the buffer
 * @param len the number of bytes to be used to represent the number in the
 *            buffer
 */
void CopyNumberToMemory(void * const pBuffer, const U32 number,
    const unsigned long len)
{
    unsigned long i;

    for (i = 0U; i < len; i++)
    {
        ((U8 *)pBuffer)[i] = (U8)((number >> (((len - i) - 1U) * 8U)) & 0xffU);
    }
}




/**
 * Copy a numerical value from a buffer, reading the MSB from the first byte of
 * the buffer.
 *
 * @param pNumber a pointer to a variable which will be updated to contain the
 *                number read from the buffer
 * @param pBuffer the buffer to read the number from
 * @param len the number of bytes used to represent the number in the buffer
 */
void CopyNumberFromMemory(U32 * const pNumber, void * const pBuffer,
    const unsigned long len)
{
    unsigned long i;

    *pNumber = 0U;
    for (i = 0U; i < len; i++)
    {
        *pNumber = (*pNumber << 8U) + ((U8 *)pBuffer)[i];
    }
}



int SizeOfFile(const char filePath[])
{
    FILE *infile;
    int size;

    infile = fopen(filePath, "rb");
    if (infile == NULL)
    {
        printf("Error: Trouble obtaining file size\n");
        return 0;
    }

    fseek(infile, 0, SEEK_END); /* seek to end of file */
    size = ftell(infile); /* get current file pointer */
    fseek(infile, 0, SEEK_SET); /* seek back to beginning of file */

    fclose(infile);

    return size;

}


