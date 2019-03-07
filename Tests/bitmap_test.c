#include "bitmap_test_util.c"
#define SIZE 5

int main (int argc, char** argv) {
	
	// Creating the bitmap
	printf ("** Creating a BitMap full of zeroes\n");
	BitMap bmap;
	uint8_t entries[SIZE];
	
	BitMap_create(&bmap, SIZE, entries);
	
	// Creating an array of blocks that simulates the HDD storage
	printf ("\n** Creating an array that simulates the HDD storage\n");
	printf ("	and filling it with selected blocks\n");
	BitMapEntryKey storage[SIZE * NUMBITS];
	for (int i = 0; i < SIZE * NUMBITS; ++i) {
		storage[i] = BitMap_blockToIndex(0);
	}
	
	BitMapEntryKey b1 	= BitMap_blockToIndex(1);
	BitMapEntryKey b2 	= BitMap_blockToIndex(2);
	BitMapEntryKey b3 	= BitMap_blockToIndex(3);
	BitMapEntryKey b7 	= BitMap_blockToIndex(7);
	BitMapEntryKey b21 	= BitMap_blockToIndex(21);
	BitMapEntryKey b23	= BitMap_blockToIndex(23);
	BitMapEntryKey b25	= BitMap_blockToIndex(25);
	BitMapEntryKey b37	= BitMap_blockToIndex(37);
	
	storage[1] 	= b1;
	storage[2] 	= b2;
	storage[3]	= b3;
	storage[7]	= b7;
	storage[21]	= b21;
	storage[23]	= b23;
	storage[25]	= b25;
	storage[37] = b37;
	
	BitMap_printStorage(storage, SIZE * NUMBITS, 1);
	
	// Changing the status of the bitmap
	printf ("\n**  Filling the bitmap from the storage array\n");
	printf ("	Should be like: [ 113 0 5 64 4 ]\n");
	BitMap_fillFromStorage (&bmap, storage, SIZE * NUMBITS);
	
	BitMap_print(&bmap);
	
	return 0;
}
