#include <iostream>
#include "bitmap.h"
#include <random>

#define BM_TEST_SIZE 256

int main(int argc, const char* argv[])
{
	s_bmPixel* pixels = (s_bmPixel*)calloc(sizeof(s_bmPixel), BM_TEST_SIZE * BM_TEST_SIZE);
	for (int i = 0; i < BM_TEST_SIZE * BM_TEST_SIZE; i++)
	{
		int random = rand() % 255 + 1;
		pixels[i] = { 0, random, random };
	}

	s_bitmap* output = bmCreate(BM_TEST_SIZE, BM_TEST_SIZE, pixels);
	bmWrite(output, "output.bmp");

	getchar();

	bmFree(output);
	delete pixels;
	return 0;
}