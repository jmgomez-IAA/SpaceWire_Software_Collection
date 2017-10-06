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
#if defined(__vxworks)
    #include <memLib.h>
#else
    #include <malloc.h>
#endif
#include <string.h>
#include <errno.h>

#include "utility.h"




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


