#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BM_HEADER_MAGIC 0x8F
#define BM_HEADER_SIZE 0x28

#define BM_FILE_SIZE_OFFSET 0x02
#define BM_DATA_OFFSET 0x0A

#define BM_WIDTH_OFFSET 0x12
#define BM_HEIGHT_OFFSET 0x16

#define BM_DATA_SIZE_OFFSET 0x22

#define BM_BITS_PER_PIXEL_OFFSET 0x1C

#define BM_COMPRESSION_OFFSET 0x1E

#define BM_COMPRESSION_METHOD_RGB 0
#define BM_COMPRESSION_METHOD_RLE8 1

#define BM_COLOR_PLANE_OFFSET 0x1A

#define BM_VERT_RES_OFFSET 0x2A
#define BM_HORIZ_RES_OFFSET 0x26

#define BM_HEADER_SIZE_OFFSET 0x0E

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
	uchar* data;
	uint32 data_size;
	uint32 data_pos;
	signed int horizontal_res;
	signed int vertical_res;
	uint32 compression;
} s_bitmap;

typedef struct s_bmPixel
{
	uchar r;
	uchar g;
	uchar b;
} s_bmPixel;


s_bitmap* bmRead(const char* string);
void bmWrite(s_bitmap* bitmap, const char* filename);
s_bitmap* bmCreate(int width, int height, s_bmPixel* pixelArray);
void bmDumpData(s_bitmap* bitmap);
void bmFree(s_bitmap* bitmap);