#include "bitmap.h"

uint32 readuint(int pos, uchar* buffer);
uint16 readushort(int pos, uchar* buffer);

void writeuint(uint32 w, int pos, uchar* buffer);
void writeushort(uint16 w, int pos, uchar* buffer);

int endian();

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

/*
writeuint(uint32 w, int pos, uchar* buffer)

Purpose: Writes w to buffer starting at positioin pos

Parameters: uint32 w: uint32 to write
			int pos: position inside of buffer to start writing
			uchar* buffer: buffer to write to
*/
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
		printf("Could not open %s\n", string);
		return BM_ERROR;
	}

	uchar header[54];
	fread(header, 1, 54, file);

	if (!header)
	{
		printf("Could not read header\n");
		return BM_ERROR;
	}

	if (header[0] + header[1] != BM_HEADER_MAGIC)
	{
		printf("File %s is not a bitmap!", string);
		return BM_ERROR;
	}

	uint32 compression = readuint(BM_COMPRESSION_OFFSET, header);

	if (compression != BM_COMPRESSION_METHOD_RGB)
	{
		printf("Bitmap uses compression of type: %d, while only compression of type 0 (RGB) is supported.\n", compression);
		return BM_ERROR;
	}

	uint32 file_size = readuint(BM_FILE_SIZE_OFFSET, header);
	uint32 data_pos = readuint(BM_DATA_OFFSET, header);
	uint32 data_size = readuint(BM_DATA_SIZE_OFFSET, header);
	uint32 width = readuint(BM_WIDTH_OFFSET, header);
	uint32 height = readuint(BM_HEIGHT_OFFSET, header);

	uint16 bits_per_pixel = readushort(BM_BITS_PER_PIXEL_OFFSET, header);

	if (data_pos == 0) data_pos = 54; //end of header
	if (data_size == 0) data_size = width * height * 3;
	if (data_size == 0)
	{
		printf("Could not compute pixel data size\n");
		return BM_ERROR;
	}

	uchar *data = new uchar[data_size];
	fread(data, 1, data_size, file);
	
	if (!data)
	{
		printf("Could not read data\n");
		return BM_ERROR;
	}

	bitmap->file_size = file_size;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->data = data;
	bitmap->data_size = data_size;
	bitmap->bits_per_pixel = bits_per_pixel;
	bitmap->data_pos = data_pos;

	fclose(file);

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

	printf("Writing bmp %s\n", filename);

	uchar header[54] = { 'B', 'M', 0 };
	uchar* data = (uchar*)bitmap->data;

	writeuint(bitmap->file_size, BM_FILE_SIZE_OFFSET, header);
	writeuint(bitmap->width, BM_WIDTH_OFFSET, header);
	writeuint(bitmap->height, BM_HEIGHT_OFFSET, header);
	writeuint(bitmap->data_pos, BM_DATA_OFFSET, header);
	writeuint(bitmap->data_size, BM_DATA_SIZE_OFFSET, header);
	writeuint(bitmap->bits_per_pixel, BM_BITS_PER_PIXEL_OFFSET, header);

	FILE *file = fopen(filename, "wb");

	if (!file)
	{
		printf("Could not open file %s\n", filename);
	}

	fwrite(header, 1, 54, file);

	fclose(file);
}