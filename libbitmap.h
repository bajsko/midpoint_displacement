//
//  bitmap.h
//  libgaussian
//
//  Created by Klas Henriksson on 2016-03-11.
//  Copyright © 2016 bajsko. All rights reserved.
//

#ifndef bitmap_h
#define bitmap_h

#define BM_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#define BM_ERROR -1
#define BM_OK 0

#define BM_FILE_HEADER_SIZE 14
#define BM_IMAGE_HEADER_SIZE 40

#define BM_SIZE_OFFSET 0x02
#define BM_PIXEL_STORAGE_OFFSET 0x0A

#define BM_IMAGE_HEADER_SIZE_OFFSET (0x0E - 14)
#define BM_WIDTH_OFFSET (0x12 - 14)
#define BM_HEIGHT_OFFSET (0x16 - 14)
#define BM_COLOR_PLANES_OFFSET (0x1A - 14)
#define BM_BITS_PER_PIXEL_OFFSET (0x1C - 14)
#define BM_COMPRESSION_OFFSET (0x1E - 14)
#define BM_RAW_DATA_SIZE_OFFSET (0x22 - 14)


typedef enum compression_mode
{
    NONE = 0,
    RLE_8 = 1,
    RLE_4 = 2,
    BITFIELDS = 3,
    JPEG = 4,
    PNG = 5,
    ALPHA_BIT_FIELDS = 6,
    CMYK = 11,
    CMYK_RLE_8 = 12,
    CMYK_RLE_4 = 13
    
} compression_mode;

typedef struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel_t;

typedef struct bitmap_t
{
    unsigned char file_header[BM_FILE_HEADER_SIZE];
    unsigned char image_header[BM_IMAGE_HEADER_SIZE];
    
    uint32_t width;
    uint32_t height;
    uint16_t bits_per_pixels;
    uint32_t file_size;
    compression_mode compression_mode;
    
    uint32_t image_header_size;
    uint32_t pixel_data_size;
    uint32_t scanline_size;
    
    uint32_t image_size;
    
    pixel_t* pixels;
} bitmap_t;

bitmap_t* bmRead(const char* filename);
bitmap_t* bmConstruct(int width, int height, pixel_t* pixelarray);
int bmWrite(bitmap_t* bitmap, const char* filename);
int bmFree(bitmap_t* bitmap);

pixel_t bmGetPixel(int x, int y, bitmap_t* bitmap);
unsigned int* bmPixelArrayToIntArray(int width, int height, pixel_t* pixelArray);
int bmSetPixelArrayFromIntArray(int width, int height, unsigned int* array, pixel_t* pixelArray);


#endif /* bitmap_h */
