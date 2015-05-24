#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#define BM_HEADER_MAGIC 0x8F
#define BM_HEADER_SIZE 0x36;

#define BM_FILE_SIZE_OFFSET 0x02
#define BM_DATA_OFFSET 0x0A

#define BM_WIDTH_OFFSET 0x12
#define BM_HEIGHT_OFFSET 0x16

#define BM_DATA_SIZE_OFFSET 0x22

#define BM_BITS_PER_PIXEL_OFFSET 0x1C

#define BM_COMPRESSION_OFFSET 0x1E

#define BM_COMPRESSION_METHOD_RGB 0
#define BM_COMPRESSION_METHOD_RLE8 1

#define BM_ERROR 0

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uchar;

typedef struct s_bitmap
{
	uint32 file_size;
	uint32 width;
	uint32 height;
	uint16 bits_per_pixel;
	void* data;
	uint32 data_size;
	uint32 data_pos;
} s_bitmap;


/*///////////////////////////////////////////////////////////////////////
bmRead(const char* string)

Purpose: Checks that specified file is a bitmap file,
then reads header and lastly data.
If any error occurs, BM_ERROR is returned

Parameters: const char* string: Path to file to be read.

Returns: s_bitmap filled with data or BM_ERROR.

Notes: This functoin does take care of endianes
*////////////////////////////////////////////////////////////////////////
s_bitmap* bmRead(const char* string);

void bmWrite(s_bitmap* bitmap, const char* filename);