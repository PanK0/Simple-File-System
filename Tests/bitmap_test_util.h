#include "../FS/bitmap.c"

static void BitMapEntryKey_print(BitMapEntryKey* entry);
static void BitMap_create(BitMap* bmap, int size, uint8_t* entries);
static void BitMap_print(BitMap* bmap);
static void BitMap_printStorage(BitMapEntryKey* storage, int storage_size, int mode);
static int	BitMap_fillFromStorage(BitMap* bmap, BitMapEntryKey* storage, int storage_size);
