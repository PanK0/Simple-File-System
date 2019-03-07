#include "../FS/bitmap.c"

// Prints the wanted entry
static void BitMapEntryKey_print(BitMapEntryKey* entry);

// Fast way to create a bitmap given an array of entries
static void BitMap_create(BitMap* bmap, int size, uint8_t* entries);

// Prints the bitmap
static void BitMap_print(BitMap* bmap);

// Prints the array called "storage" that simulates a disk with free or occupied blocks
// if mode == 1 then only prints occupied blocks
static void BitMap_printStorage(BitMapEntryKey* storage, int storage_size, int mode);

// Fills the bitmap bmap with the blocks of a "storage array"
static int	BitMap_fillFromStorage(BitMap* bmap, BitMapEntryKey* storage, int storage_size);
