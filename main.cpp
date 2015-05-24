#include <iostream>
#include "bitmap.h"

using namespace std;

int main(int argc, const char* argv[])
{
	if (argc < 2)
		return 1;

	s_bitmap *bitmap = bmRead(argv[1]);

	if (bitmap == BM_ERROR)
	{
		getchar();
		return 0;
	}

	printf("BITMAP INFO on bitmap %s\n", argv[1]);
	printf("bitmap total file size: %d\n", bitmap->file_size);
	printf("Bitmap width: %d\n", bitmap->width);
	printf("Bitmap height: %d\n", bitmap->height);
	printf("Bitmap data size: %d\n", bitmap->data_size);
	printf("Bitamp bits per pixel: %d\n", bitmap->bits_per_pixel);

	bmWrite(bitmap, "created_bitmap.bmp");

	getchar();

	return 0;
}