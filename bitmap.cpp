#include "bitmap.h"

uint32 readuint(int pos, uchar* buffer);
uint16 readushort(int pos, uchar* buffer);

void writeuint(uint32 w, int pos, uchar* buffer);
void writeushort(uint16 w, int pos, uchar* buffer);

int endian();

int roundUp(int num, int multiple);

/*/////////////////////////////////////////
endian()

Purpose: Checks wheter this system uses 
			big endian or little endian

Returns: 1 if little endian, 0 if big endian.
*///////////////////////////////////////////
int endian()
{
	int x = 1;

	if (*(char *)&x == 1) return 1;

	return 0;
}

/*/////////////////////////////////////////////////////////////////////
readuint(int pos, char* buffer);

Purpose: Reads 4 chars starting from adress pos,
			then merges them to an integer

Parameters: pos: Starting read position.
			char* buffer: buffer to read from.

Returns: uint32 with read data

*///////////////////////////////////////////////////////////////////////
uint32 readuint(int pos, uchar* buffer)
{
	uchar b1 = buffer[pos];
	uchar b2 = buffer[pos + 1];
	uchar b3 = buffer[pos + 2];
	uchar b4 = buffer[pos + 3];

	uint32 final = 0;

	if (endian() == 1)
	{
		final |= (b4 << 24);
		final |= (b3 << 16);
		final |= (b2 << 8);
		final |= (b1);
	}
	else
	{
		final |= (b1 << 24);
		final |= (b2 << 16);
		final |= (b3 << 8);
		final |= (b4);
	}

	return final;
}

/*/////////////////////////////////////////////////////////////////////
readuint(int pos, char* buffer);

Purpose: Reads 2 chars starting from adress pos,
then merges them to a short

Parameters: pos: Starting read position.
			char* buffer: buffer to read from.

Returns: uint16 with read data

*///////////////////////////////////////////////////////////////////////
uint16 readushort(int pos, uchar* buffer)
{
	uchar b1 = buffer[pos];
	uchar b2 = buffer[pos + 1];

	uint16 final = 0;

	if (endian() == 1)
	{
		final |= (b2 << 8);
		final |= b1;
	}
	else
	{
		final |= (b1 << 8);
		final |= b2;
	}

	return final;
}

/*///////////////////////////////////////////////////////////////
writeuint(uint32 w, int pos, uchar* buffer)

Purpose: Writes w to buffer starting at position pos

Parameters: uint32 w: uint32 to write
			int pos: position inside of buffer to start writing
			uchar* buffer: buffer to write to
*////////////////////////////////////////////////////////////////
void writeuint(uint32 w, int pos, uchar* buffer)
{
	uchar c1 = w;
	uchar c2 = (w >> 8);
	uchar c3 = (w >> 16);
	uchar c4 = (w >> 24);

	buffer[pos] = c1;
	buffer[pos+1] = c2;
	buffer[pos+2] = c3;
	buffer[pos+3] = c4;
}

/*
writeushort(uint16 w, int pos, uchar* buffer)

Purpose: Writes w to buffer starting at positioin pos

Parameters: uint16 w: uint16 to write
int pos: position inside of buffer to start writing
uchar* buffer: buffer to write to
*/
void writeushort(uint16 w, int pos, uchar* buffer)
{
	uchar c1 = w;
	uchar c2 = (w >> 8);

	buffer[pos] = c1;
	buffer[pos + 1] = c2;
}

/*////////////////////////////////////////////////////////
roundUp(int num int multiple)

Purpose: rounds up num to the nearest multiple of multiple

Returns: rounded up num
*/////////////////////////////////////////////////////////
int roundUp(int num, int multiple)
{
	int remainder = num % multiple;

	return num + multiple - remainder;
}

/*///////////////////////////////////////////////////////////////////////
bmRead(const char* string)

Purpose: Checks that specified file is a bitmap file,
			then reads header and lastly data.
			If any error occurs, BM_ERROR is returned

Parameters: const char* string: Path to file to be read.

Returns: s_bitmap filled with data or BM_ERROR.

*////////////////////////////////////////////////////////////////////////
s_bitmap* bmRead(const char* string)
{
	
	s_bitmap* bitmap = (s_bitmap*)calloc(sizeof(s_bitmap), 1);

	FILE *file = fopen(string, "rb");
	if (!file)
	{
		printf("bmRead: Could not open %s\n", string);
		return BM_ERROR;
	}

	uchar header[54];
	fread(header, 1, 54, file);

	if (!header)
	{
		printf("bmRead: Could not read header\n");
		return BM_ERROR;
	}

	if (header[0] + header[1] != BM_HEADER_MAGIC)
	{
		printf("bmRead: File %s is not a bitmap!", string);
		return BM_ERROR;
	}

	uint32 compression = readuint(BM_COMPRESSION_OFFSET, header);

	if (compression != BM_COMPRESSION_METHOD_RGB)
	{
		printf("bmRead: Bitmap uses compression of type: %d, while only compression of type 0 (RGB) is supported.\n", compression);
		return BM_ERROR;
	}

	uint32 file_size = readuint(BM_FILE_SIZE_OFFSET, header);
	uint32 data_pos = readuint(BM_DATA_OFFSET, header);
	uint32 data_size = readuint(BM_DATA_SIZE_OFFSET, header);
	signed int width = (signed int)readuint(BM_WIDTH_OFFSET, header);
	signed int height = (signed int)readuint(BM_HEIGHT_OFFSET, header);

	signed int vert_res = readuint(BM_VERT_RES_OFFSET, header);
	signed int horiz_res = readuint(BM_HORIZ_RES_OFFSET, header);

	uint16 bits_per_pixel = readushort(BM_BITS_PER_PIXEL_OFFSET, header);

	if (data_pos == 0) data_pos = 54; //end of header

	if (data_size == 0) data_size = width * height * 3;
	if (data_size == 0)
	{
		printf("bmRead: Could not compute pixel data size\n");
		return BM_ERROR;
	}

	uchar *data = new uchar[data_size];
	if (!data)
	{
		printf("bmRead: Could not allocate data(0) memory!\n");
		return BM_ERROR;
	}

	fread(data, 1, data_size, file);

	bitmap->data = (uchar*)malloc(data_size * sizeof(uchar));
	if (!bitmap->data)
	{
		printf("bmRead: Could not allocate data(1) memory!\n");
		return BM_ERROR;
	}

	memcpy(bitmap->data, data, data_size * sizeof(uchar));

	bitmap->file_size = file_size;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->data_size = data_size;
	bitmap->bits_per_pixel = bits_per_pixel;
	bitmap->data_pos = data_pos;
	bitmap->vertical_res = vert_res;
	bitmap->horizontal_res = horiz_res;
	bitmap->compression = compression;

	fclose(file);

	delete data;
	return bitmap;
}

/*//////////////////////////////////////////////////////////////////////////
bmWrite(s_bitmap* bitmap, const char* filename)

Purpose: Writes specified bitmap to specifie file.

Parameters: s_bitmap* bitmap: Bitmap to write
			const char* filename: File to write to. (creates if not found, overwrites if found)

*/////////////////////////////////////////////////////////////////////////////
void bmWrite(s_bitmap* bitmap, const char* filename)
{

	printf("bmWrite: Writing bmp %s\n", filename);

	uchar header[54] = { 'B', 'M', 0 };
	uchar* data = (uchar*)bitmap->data;

	writeuint(bitmap->file_size, BM_FILE_SIZE_OFFSET, header);
	writeuint(BM_HEADER_SIZE, BM_HEADER_SIZE_OFFSET, header);
	writeuint(bitmap->width, BM_WIDTH_OFFSET, header);
	writeuint(bitmap->height, BM_HEIGHT_OFFSET, header);
	writeuint(bitmap->data_pos, BM_DATA_OFFSET, header);
	writeuint(bitmap->data_size, BM_DATA_SIZE_OFFSET, header);

	writeushort(bitmap->bits_per_pixel, BM_BITS_PER_PIXEL_OFFSET, header);
	writeushort(0x1, BM_COLOR_PLANE_OFFSET, header);

	writeuint(bitmap->vertical_res, BM_VERT_RES_OFFSET, header);
	writeuint(bitmap->horizontal_res, BM_HORIZ_RES_OFFSET, header);

	FILE *file = fopen(filename, "wb");

	if (!file)
	{
		printf("bmWrite: Could not open file %s\n", filename);
		return;
	}

	fwrite(header, 1, 54, file);
	fwrite(data, 1, bitmap->data_size, file);

	fclose(file);

	printf("bmWrite: Finished!\n");
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////
bmCreate(int width, int height, s_bmPixel* pixelArray)

Purpose: Creates a new bitmap with specified width & height
			default header values:	compression: BM_COMPRESSION_METHOD_RGB
									bits_per_pixel: 24 (only one supported atm)
									horizontal_res: 0xc40e
									vertical_res: 0xc40e

Parameters:	int width: width of bmp
			int height: height of bmp
			s_bmPixel* pixelArray: array of pixels to be used

Returns s_bitmap* filled with data or BM_ERROR.

*/////////////////////////////////////////////////////////////////////////////////////////////////
s_bitmap* bmCreate(int width, int height, s_bmPixel* pixelArray)
{
	printf("bmCreate: Creating bitmap..\n");
	s_bitmap* bitmap = (s_bitmap*)calloc(sizeof(s_bitmap), 1);

	bitmap->width = width;
	bitmap->height = height;
	bitmap->data_pos = 54;
	bitmap->compression = BM_COMPRESSION_METHOD_RGB;
	bitmap->bits_per_pixel = 24;
	bitmap->horizontal_res = 0xC40E;
	bitmap->vertical_res = 0xC40E;

	int rowSize = ((bitmap->bits_per_pixel * width + 31) / 32) * 4;
	printf("bmCreate: row size: %d\n", rowSize);
	int pixelArraySize = rowSize * height;

	bitmap->file_size = BM_HEADER_SIZE + pixelArraySize;
	bitmap->data_size = pixelArraySize;

	uchar* data = (uchar*)malloc(bitmap->data_size);
	if (!data)
	{
		printf("bmCreate: Could not allocate data(0) memory!\n");
		return BM_ERROR;
	}

	bitmap->data = (uchar*)malloc(bitmap->data_size);
	if (!bitmap->data)
	{
		printf("bmCreate: Could not allocate data(1) memory!\n");
		return BM_ERROR;
	}

	//pixel algorithm
	//TODO: fix padding if needed, is it ever?
	int nescessaryPadding = 0;
	if (rowSize % 4 != 0)
	{
		nescessaryPadding = roundUp(rowSize, 4) - rowSize;
		printf("bmCreate: row needed padding %d\n", nescessaryPadding);
	}

	int bytes_per_pixel = bitmap->bits_per_pixel / 8;

	int y = 0;
	while (y < height)
	{
		for (int i = 0; i < rowSize; i += bytes_per_pixel)
		{
			int index = ((i / 3) + (y * width));
			s_bmPixel pixel = pixelArray[index];

			data[(i) + y*rowSize] = pixel.r;
			data[(i + 1) + y*rowSize] = pixel.g;
			data[(i + 2) + y*rowSize] = pixel.b;
		}
		y++;
	}
	//end

	memcpy(bitmap->data, data, pixelArraySize * sizeof(uchar));

	delete data;

	printf("bmCreate: Finished!\n");

	return bitmap;
}

void bmDumpData(s_bitmap* bitmap)
{
	printf("---------------------------\n");
	printf("Bitmap dump data report\n");
	printf("Total file size (bytes): %d\n", bitmap->file_size);
	printf("Bitmap data size (bytes): %d\n", bitmap->data_size);
	printf("Vertical res: 0x%X\n", bitmap->vertical_res);
	printf("Horizontal res: 0x%X\n", bitmap->horizontal_res);
	printf("Compression mode: %d\n", bitmap->compression);
	printf("-------------------------------\n");
}

void bmFree(s_bitmap* bitmap)
{
	delete bitmap->data;
	delete bitmap;
}