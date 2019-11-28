/**
 * \file utility.c
 *
 * \brief Declarations of general utility functions for the STAR-System Test
 *        program.
 *
 * \author STAR-Dundee Ltd.\n
 *         STAR House\n
 *         166 Nethergate\n
 *         Dundee, DD1 4EE\n
 *         Scotland, UK\n
 *         e-mail: support@star-dundee.com
 *
 * Declarations of utility functions, and definitions of macros, used by the
 * STAR-System Test program.
 *
 * Copyright &copy; 2013 STAR-Dundee Ltd
 */

#include <time.h>
#include "star-api.h"
#include "rmap_packet_library.h"

#ifndef UTILITYFILE_H
#define UTILITYFILE_H

#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))

#define DATA_TYPE_0             0
#define DATA_TYPE_1             1
#define DATA_TYPE_RANDOM        2
#define DATA_TYPE_COUNT         3
#define DATA_TYPE_NOT_COUNT     4



#ifdef _WIN32
    #include "windows.h"

    #define SLEEP(milliseconds) Sleep(milliseconds);

#else

    #include <unistd.h>
    #define SLEEP(milliseconds) usleep(milliseconds * 1000);

#endif


/**
 * @brief Prints a SpaceWire RMAP Reply address
*/

typedef struct rmap_reply_header_t
{
  unsigned char  init_address;          //  Byte 1
  unsigned char  protocol_id;           //  Byte 2
  unsigned char  instruction_field;     //  Byte 3
  unsigned char  cmd_status;            //  Byte 4
  unsigned char  target_addr;           //  Byte 5
  unsigned short  tans_ident;           //  Byte 6-7
  unsigned char  reserved_1;           //  Byte 8
  unsigned int  data_length;           //  Byte 9,10,11
  unsigned char  header_crc;             //  Byte 12
}rmap_reply_header_t;


typedef struct rmap_reply_data_t
{
  unsigned int data_length;
  unsigned char *data_chunk;
  unsigned char data_crc;
}rmap_reply_data_t;

#define HEADER_OFFSET 0
#define RPLY_PACKET_DATA_OFFSET 12

unsigned int reverse_4bytes_array(unsigned int lsb_integer);
void display_packet_type(RMAP_PACKET_TYPE type);
void display_packet_status(RMAP_STATUS status);
void dumpPacket( STAR_SPACEWIRE_PACKET * StreamItemPacket);
unsigned long printRxPackets(STAR_TRANSFER_OPERATION * const pTransferOp);
unsigned long dumpTransferOperation(STAR_TRANSFER_OPERATION * const pTransferOp);
void printReply(STAR_SPACEWIRE_PACKET * StreamItemPacket);
unsigned int random32(void);
unsigned int random16(void);

void FillBuffer(unsigned int *pBuffer, unsigned long size, int dataType);

void FillBufferRandomChar(char * const pBuffer, const unsigned int size,
    const int dataType);

int file_read(const char fname[], char * const pBuffer,
    unsigned int * const byteSize);

void debug_print(char *s);

unsigned long BufferCompareChar(const char * const pBuffer1,
    const char * const pBuffer2, const unsigned long size);

int readSpaceWireAddress(STAR_SPACEWIRE_ADDRESS** pAddress);

int SizeOfFile(const char filePath[]);

int file_read_chunk(const char fname[], char * const pBuffer, const long offset,
    const unsigned int size);

int file_append_chunk(const unsigned char * const pData, const long dataSize,
const char fname[]);

void CopyNumberToMemory(void * const pBuffer, const U32 number,
    const unsigned long len);

void CopyNumberFromMemory(U32 * const pNumber, void * const pBuffer,
    const unsigned long len);















#endif


