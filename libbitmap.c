//
//  bitmap.c
//  libgaussian
//
//  Created by Klas Henriksson on 2016-03-11.
//  Copyright © 2016 bajsko. All rights reserved.
//

#include "libbitmap.h"

static void Report(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static uint32_t ReadInt(unsigned char* buffer, int offset)
{
    uint32_t ret = 0;
    ret |= (buffer[offset+3] << 24);
    ret |= (buffer[offset+2] << 16);
    ret |= (buffer[offset+1] << 8);
    ret |= (buffer[offset]);
    
    return ret;
}

static uint16_t ReadShort(unsigned char* buffer, int offset)
{
    uint32_t ret = 0;
    ret = (buffer[offset + 1] << 8 | buffer[offset]);
    
    return ret;
}

//care: buffer overflow!
static void WriteInt(unsigned char* buffer, int offset, uint32_t num)
{
    buffer[offset+3] = ((num & 0xFF000000) >> 24);
    buffer[offset+2] = ((num & 0xFF0000) >> 16);
    buffer[offset+1] = ((num & 0xFF00) >> 8);
    buffer[offset] = ((num));
}

static void WriteShort(unsigned char* buffer, int offset, uint16_t num)
{
    buffer[offset+1] = ((num & 0xFF00) >> 8);
    buffer[offset] = ((num & 0xFF));
}

pixel_t bmGetPixel(int x, int y, bitmap_t* bitmap)
{
    pixel_t ret = { 0 };
    if(x < 0 || x >= bitmap->width || y < 0 || y >= bitmap->height)
        return ret;
    
    ret = bitmap->pixels[x + y * bitmap->width];
    
    return ret;
}

bitmap_t* bmRead(const char* filename)
{
    printf("bmRead: reading %s\n", filename);
    bitmap_t* ret;
    
    FILE* file = fopen(filename, "rb");
    
    if(!file)
    {
        Report("bmRead: fopen() failed!\n");
        return NULL;
    }
    
    unsigned char file_header[BM_FILE_HEADER_SIZE];
    fread(file_header, 1, BM_FILE_HEADER_SIZE, file);
    
    if(file_header[0] != 'B' || file_header[1] != 'M')
    {
        Report("bmRead: file is not a bitmap!\n");
        return NULL;
    }

    
    uint32_t file_size = ReadInt(file_header, BM_SIZE_OFFSET);
    uint32_t pixel_storage_offset = ReadInt(file_header, 0xA);
    
    unsigned char image_header[BM_IMAGE_HEADER_SIZE];
    fread(image_header, 1, BM_IMAGE_HEADER_SIZE, file);
    
    uint32_t header_size = ReadInt(image_header, BM_IMAGE_HEADER_SIZE_OFFSET);
    uint32_t width = ReadInt(image_header, BM_WIDTH_OFFSET);
    uint32_t height = ReadInt(image_header, BM_HEIGHT_OFFSET);
    uint16_t bits_per_pixel = ReadShort(image_header, BM_BITS_PER_PIXEL_OFFSET);
    uint32_t compression = ReadInt(image_header, BM_COMPRESSION_OFFSET);
    uint32_t image_size = ReadInt(image_header, BM_RAW_DATA_SIZE_OFFSET);
    
    ret = malloc(sizeof(bitmap_t));
    if(!ret)
    {
        Report("bmRead: Could not allocate enough memory for bitmap!\n");
        return NULL;
    }
    
    memcpy(ret->file_header, file_header, BM_FILE_HEADER_SIZE);
    memcpy(ret->image_header, image_header, BM_IMAGE_HEADER_SIZE);
    
    ret->width = width;
    ret->height = height;
    ret->bits_per_pixels = bits_per_pixel;
    ret->compression_mode = (compression_mode)compression;
    ret->image_header_size = header_size;
    ret->file_size = file_size;
    
#ifdef BM_DEBUG
    printf("Bitmap %s\n", filename);
    printf("Width: %d\nHeight: %d\nBits per pixel: %d\nCompression mode:%d\nImage header size:%d\nPixel offset:%d\nFile size: %d\n",
           width, height, bits_per_pixel, compression, header_size, pixel_storage_offset, file_size);
#endif
    
    //seek to data
    if(fseek(file, pixel_storage_offset, SEEK_SET) != 0)
    {
        Report("bmRead: Could not set stream position!\n");
        return NULL;
    }
    
    uint32_t row_size = ((bits_per_pixel * width + 31) / 32) * 4;
    uint32_t data_size = image_size;
    
    unsigned char* pixel_data = malloc(sizeof(unsigned char) * data_size);
    fread(pixel_data, 1, data_size, file);
    fclose(file);
    
    pixel_t* pixelArray = malloc(sizeof(pixel_t) * width * height);
    if(!pixelArray)
    {
        Report("bmRead: Could not allocate memory for pixel array!\n");
        return NULL;
    }
    
    int unusedBytes = row_size - (width * (bits_per_pixel/8));
    
    for(int x = 0; x < (row_size-unusedBytes); x+= (bits_per_pixel/8))
    {
        for(int y = 0; y < height; y++)
        {
            int index = (x/3) + y * width;
            
            char b = pixel_data[x + y * row_size];
            char g = pixel_data[(x+1) + y * row_size];
            char r = pixel_data[(x+2) + y * row_size];
            
            pixel_t pixel;
            pixel.r = r;
            pixel.g = g;
            pixel.b = b;
            
            pixelArray[index] = pixel;
        }
    }
    
    ret->pixels = pixelArray;
    ret->pixel_data_size = data_size;
    ret->scanline_size = row_size;
    
    free(pixel_data);
    
    return ret;
}

bitmap_t* bmConstruct(int width, int height, pixel_t* pixelarray)
{
    printf("bmConstruct: constructing bitmap...\n");
    
    bitmap_t* ret = malloc(sizeof(bitmap_t));
    unsigned char file_header[BM_FILE_HEADER_SIZE];
    unsigned char image_header[BM_IMAGE_HEADER_SIZE];
    
    file_header[0] = 'B';
    file_header[1] = 'M';
    WriteInt(file_header, BM_PIXEL_STORAGE_OFFSET, 54);
    WriteShort(image_header, BM_COLOR_PLANES_OFFSET, 1);
    WriteShort(image_header, BM_BITS_PER_PIXEL_OFFSET, 24);
    WriteInt(image_header, BM_COMPRESSION_OFFSET, 0);
    WriteInt(image_header, BM_WIDTH_OFFSET, width);
    WriteInt(image_header, BM_HEIGHT_OFFSET, height);
    WriteInt(image_header, BM_IMAGE_HEADER_SIZE_OFFSET, BM_IMAGE_HEADER_SIZE);
    
    memcpy(ret->file_header, file_header, BM_FILE_HEADER_SIZE);
    memcpy(ret->image_header, image_header, BM_IMAGE_HEADER_SIZE);
    
    ret->width = width;
    ret->height = height;
    ret->bits_per_pixels = 24;
    ret->compression_mode = 0;
    ret->scanline_size = ((ret->bits_per_pixels * width + 31)/32) * 4;
    
    pixel_t* pixels = malloc(sizeof(pixel_t) * width * height);
    for(int i = 0; i < width * height; i++)
    {
        pixel_t pixel;
        pixel = pixelarray[i];
        pixels[i] = pixel;
    }
    
    ret->pixels = pixels;
    ret->image_size = ret->scanline_size * height;
    ret->pixel_data_size = ret->image_size;
    ret->image_header_size = BM_IMAGE_HEADER_SIZE;
    
    printf("bmConstruct: Constructed bitmap!\n");
    
    return ret;
}

int bmWrite(bitmap_t* bitmap, const char* filename)
{
    printf("bmWrite: writing bitmap to disk...\n");
    
    FILE *file = fopen(filename, "w");
    
    if(!file)
    {
        Report("bmWrite: Could not create file for writing!\n");
        return BM_ERROR;
    }
    
    unsigned char file_header[BM_FILE_HEADER_SIZE];
    unsigned char image_header[BM_IMAGE_HEADER_SIZE];
    
    unsigned char header[BM_FILE_HEADER_SIZE + BM_IMAGE_HEADER_SIZE];
    
    file_header[0] = 'B';
    file_header[1] = 'M';
    WriteInt(file_header, BM_PIXEL_STORAGE_OFFSET, 54);
    WriteShort(image_header, BM_COLOR_PLANES_OFFSET, 1);
    WriteShort(image_header, BM_BITS_PER_PIXEL_OFFSET, bitmap->bits_per_pixels);
    WriteInt(image_header, BM_COMPRESSION_OFFSET, bitmap->compression_mode);
    WriteInt(image_header, BM_WIDTH_OFFSET, bitmap->width);
    WriteInt(image_header, BM_HEIGHT_OFFSET, bitmap->height);
    WriteInt(image_header, BM_RAW_DATA_SIZE_OFFSET, bitmap->image_size);
    WriteInt(image_header, BM_IMAGE_HEADER_SIZE_OFFSET, BM_IMAGE_HEADER_SIZE);
    
    unsigned char* data = malloc(sizeof(unsigned char) * bitmap->pixel_data_size);
    //memset(data, 0x00, bitmap->pixel_data_size);
    
   for(int x = 0; x < bitmap->scanline_size; x+= (bitmap->bits_per_pixels / 8))
    {
        for(int y = 0; y < bitmap->height; y++)
        {
            int rx = (x/3);
            pixel_t pixel = bmGetPixel(rx, y, bitmap);
            
            data[(x) + (y) * bitmap->scanline_size] = pixel.b;
            data[(x + 1) + (y) * bitmap->scanline_size] = pixel.g;
            data[(x + 2) + (y) * bitmap->scanline_size] = pixel.r;
        }
    }
    
    
    uint32_t totalSize = BM_FILE_HEADER_SIZE + BM_IMAGE_HEADER_SIZE + bitmap->pixel_data_size;
    WriteInt(file_header, BM_SIZE_OFFSET, totalSize);
    
    
    memcpy(header, file_header, BM_FILE_HEADER_SIZE);
    memcpy((header+BM_FILE_HEADER_SIZE), image_header, BM_IMAGE_HEADER_SIZE);
    
    fwrite(header, 1, BM_FILE_HEADER_SIZE + BM_IMAGE_HEADER_SIZE, file);
    
    //seek to data position
    /*if(fseek(file, 54 , SEEK_SET) != 0)
    {
        Report("bmWrite: Could not set stream position!\n");
        return BM_ERROR;
    }*/
    
    fwrite(data, 1, bitmap->pixel_data_size, file);
    fclose(file);
    
    printf("bmWrite complete!\n");
    
    free(data);
    
    return BM_OK;
}

unsigned int* bmPixelArrayToIntArray(int width, int height, pixel_t* pixelArray)
{
    uint32_t* array = malloc(sizeof(uint32_t) * width * height);
    
    if(!array)
        return NULL;
    
    for(int i = 0; i < width * height; i++)
    {
        pixel_t pixel = pixelArray[i];
        uint32_t intPixel = 0;
        
        unsigned char r = pixel.r;
        unsigned char g = pixel.g;
        unsigned char b = pixel.b;
        
        intPixel |= (r << 24);
        intPixel |= (g << 16);
        intPixel |= (b << 8);
        
        array[i] = intPixel;
    }
    
    return array;
}

uint32_t bmSetPixelArrayFromIntArray(int width, int height, uint32_t* array, pixel_t* pixelArray)
{
    for(int i = 0; i < width * height; i++)
    {
        unsigned int intPixel = array[i];
        
        pixelArray[i].r = (intPixel & 0xFF000000) >> 24;
        pixelArray[i].g = (intPixel & 0xFF0000) >> 16;
        pixelArray[i].b = (intPixel & 0xFF00) >> 8;
    }
    
    return BM_OK;
}

int bmFree(bitmap_t* bitmap)
{
    free(bitmap->pixels);
    free(bitmap);
    
    return BM_OK;
}
