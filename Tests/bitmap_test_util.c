#include "bitmap_test_util.h"
#include <stdio.h>

// Prints the wanted entry
void BitMapEntryKey_print(BitMapEntryKey* entry) {
	printf ("[ entry_num: %d, ", entry->entry_num);
	printf ("array cell: %d, ", (entry->entry_num) / NUMBITS);
	printf ("bit_num: %d ]\n", entry->bit_num);
	
}

// Fast way to create a bitmap given an array of entries
void BitMap_create(BitMap* bmap, int size, uint8_t* entries) {
	bmap->num_bits = size;
	bmap->entries = entries;
	for (int i = 0; i < size; i++) {
		bmap->entries[i] = 0;
	}	
}

// Prints the bitmap
void BitMap_print(BitMap* bmap) {
	printf ("Size: %d\n", bmap->num_bits);
	int size = bmap->num_bits;
	printf ("[ ");
	for (int i = 0; i < size; i++) {
		printf("%d ", bmap->entries[i]);
	}
	printf ("]\n");
}

// Prints the array called "storage" that simulates a disk with free or occupied blocks
// if mode == 1 then only prints occupied blocks
void BitMap_printStorage(BitMapEntryKey* storage, int storage_size, int mode) {
	printf ("**	Printing storage. . . \n");
	if (!mode) {
		for (int i = 0; i < storage_size; ++i) {
			printf ("[ storage_index = %d, entry_num = %d, bit_num = %d ]\n", i, storage[i].entry_num, storage[i].bit_num);
		}
	}
	else if (mode) {
		for (int i = 0; i < storage_size; ++i) {
			if (storage[i].entry_num != 0 || storage[i].bit_num != 0) {
				printf ("[ storage_index = %d, entry_num = %d, bit_num = %d ]\n", i, storage[i].entry_num, storage[i].bit_num);
			}
		}
	}
}

// Fills the bitmap bmap with the blocks of a "storage array"
int BitMap_fillFromStorage(BitMap* bmap, BitMapEntryKey* storage, int storage_size) {
	int ret, status = 0;
	for (int i = 0; i < storage_size; ++i) {
		if (storage[i].entry_num != 0 || storage[i].bit_num != 0) status = 1;
		else status = 0;
		ret = BitMap_set(bmap, i, status);
		if (ret == ERROR_RESEARCH_FAULT) return ERROR_RESEARCH_FAULT;
	}
	return 1;
}
