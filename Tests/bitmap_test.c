#include "bitmap_test_util.c"
#define SIZE 512

int main (int argc, char** argv) {
	
	printf ("HELLO WORLD!\n");
	
	// Creating the bitmap
	printf ("Creating a BitMap full of zeroes\n");
	BitMap bmap;
	uint8_t entries[SIZE];
	
	BitMap_create(&bmap, SIZE, entries);
	BitMap_print(&bmap);	
	
	// Creating the list of blocks
	
	// Changing the status of the bitmap
	
	return 0;
}
