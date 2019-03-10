#include "bitmap_test_util.c"
#define SIZE 5

int main (int argc, char** argv) {
	
	// Creating the bitmap
	printf ("** Creating a BitMap full of zeroes\n");
	BitMap bmap;
	uint8_t entries[SIZE];
	
	BitMap_create(&bmap, SIZE, entries);
	
	// Creating an array of blocks that simulates the HDD storage - testing BitMap_blockToIndex()
	printf ("\n** Creating an array that simulates the HDD storage - testing BitMap_blockToIndex()\n");
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
	
	// Changing the status of the bitmap - testing BitMap_set() 
	printf ("\n**  Filling the bitmap from the storage array - testing BitMap_set()\n");
	printf ("	Should be like: [ 113 0 5 64 4 ]\n");
	printf ("	For machines  :	[ 01110001 00000000 00000101 01000000 00000100 ]\n");
	BitMap_fillFromStorage (&bmap, storage, SIZE * NUMBITS);
	
	printf ("Number of free blocks : %d\n   - testing BitMap_getFreeBlocks()", BitMap_getFreeBlocks(&bmap));
	BitMap_print(&bmap);
	
	// Is Bit set? - testing BitMap_isBitSet()
	printf ("\n**  Is bit set? - testing BitMap_isBitSet()");
	printf ("  Given the position of a block in the storage tells if that bit is set or not\n");
	
	int isset;
	isset = BitMap_isBitSet(&bmap, 0);
	printf ("Block 0 in the storage is %d, should be: 0\n", isset);
	
	isset = BitMap_isBitSet(&bmap, 1);
	printf ("Block 1 in the storage is %d, should be: 1\n", isset);
	
	isset = BitMap_isBitSet(&bmap, 15);
	printf ("Block 15 in the storage is %d, should be: 0\n", isset);
	
	isset = BitMap_isBitSet(&bmap, 23);
	printf ("Block 23 in the storage is %d, should be: 1\n", isset);
	
	isset = BitMap_isBitSet(&bmap, 37);
	printf ("Block 37 in the storage is %d, should be: 1\n", isset);
	
	// Get a bit - testing BitMap_get()
	printf ("\n**  Looking for bits - testing BitMap_get()\n");
	int bit_index;
	
	bit_index = BitMap_get(&bmap, 0 , 1);
	printf ("The global index of the 1st bit with status 1 at entry: 0 is:  %d, should be: 1\n", bit_index);
	bit_index = BitMap_get(&bmap, 0 , 0);
	printf ("The global index of the 1st bit with status 0 at entry: 0 is:  %d, should be: 0\n", bit_index);
	
	bit_index = BitMap_get(&bmap, 1 , 1);
	printf ("The global index of the 1st bit with status 1 at entry: 1 is:  %d, should be: 21\n", bit_index);
	bit_index = BitMap_get(&bmap, 1 , 0);
	printf ("The global index of the 1st bit with status 0 at entry: 1 is:  %d, should be: 8\n", bit_index);
	
	bit_index = BitMap_get(&bmap, 2 , 1);
	printf ("The global index of the 1st bit with status 1 at entry: 2 is:  %d, should be: 21\n", bit_index);
	bit_index = BitMap_get(&bmap, 2 , 0);
	printf ("The global index of the 1st bit with status 1 at entry: 2 is:  %d, should be: 16\n", bit_index);
	
	bit_index = BitMap_get(&bmap, 3 , 1);
	printf ("The global index of the 1st bit with status 1 at entry: 3 is:  %d, should be: 25\n", bit_index);
	bit_index = BitMap_get(&bmap, 3 , 0);
	printf ("The global index of the 1st bit with status 1 at entry: 3 is:  %d, should be: 24\n", bit_index);
	
	bit_index = BitMap_get(&bmap, 4 , 1);
	printf ("The global index of the 1st bit with status 1 at entry: 4 is:  %d, should be: 37\n", bit_index);
	bit_index = BitMap_get(&bmap, 4 , 0);
	printf ("The global index of the 1st bit with status 1 at entry: 4 is:  %d, should be: 32\n", bit_index);
	
	// Finding the storage index of the block corresponding to a bit - testing BitMap_indexToBlock()
	printf ("\n**  Finding blocks given a bit - testing BitMap_indexToBlock()\n");
	int entry0 = 0, entry1 = 1, entry3 = 3;
	uint8_t bit0 = 0, bit1 = 1, bit7 = 7;
	int linear_index;
	
	linear_index = BitMap_indexToBlock(entry0, bit0);
	printf ("Linear index of entry %d at bit in pos %d is : %d\n", entry0, bit0, linear_index);
	
	linear_index = BitMap_indexToBlock(entry0, bit1);
	printf ("Linear index of entry %d at bit in pos %d is : %d\n", entry0, bit1, linear_index);
	
	linear_index = BitMap_indexToBlock(entry0, bit7);
	printf ("Linear index of entry %d at bit in pos %d is : %d\n", entry0, bit7, linear_index);
	
	linear_index = BitMap_indexToBlock(entry1, bit7);
	printf ("Linear index of entry %d at bit in pos %d is : %d\n", entry1, bit7, linear_index);
	
	linear_index = BitMap_indexToBlock(entry3, bit1);
	printf ("Linear index of entry %d at bit in pos %d is : %d\n", entry3, bit1, linear_index);
	
	
	
	return 0;
}
