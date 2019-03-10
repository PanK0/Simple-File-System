#pragma once
#include "../FS/bitmap.c"

// Prints the wanted entry
void BitMapEntryKey_print(BitMapEntryKey* entry);

// Fast way to create a bitmap given an array of entries
void BitMap_create(BitMap* bmap, int size, uint8_t* entries);

// Prints the bitmap
void BitMap_print(BitMap* bmap);

// Prints the array called "storage" that simulates a disk with free or occupied blocks
// if mode == 1 then only prints occupied blocks
void BitMap_printStorage(BitMapEntryKey* storage, int storage_size, int mode);

// Fills the bitmap bmap with the blocks of a "storage array"
int	BitMap_fillFromStorage(BitMap* bmap, BitMapEntryKey* storage, int storage_size);
