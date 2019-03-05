#include bitmap.h

// converts a block index to an index in the array,
// and a uint8_t that indicates the offset of the bit inside the array.
BitMapEntryKey BitMap_blockToIndex(int num) {
	BitMapEntryKey entry_key;
	entry_key.entry_num = num / NUMBITS;
	entry_key.bit_num = num % NUMBITS;
	return entry_key;
	
}

// converts a bit to a linear index
int BitMap_indexToBlock(int entry, uint8_t bit_num) {
	int num = (entry * NUMBITS) + bit_num;
	return num;
}

// returns the pos of the first bit equal to status in number num
// returns -1 in case of bit not found
int BitMap_check(uint8_t num, int status) {
	if (num < 0) return ERROR_RESEARCH_FAULT;
	int i = 7;
	while (i >= 0) {
		if (status != 0 && (num & (1 << i))) {
			return i;			
		}
		else if (status == 0 && !(num & (1 << i))) {
			return i;
		}	
	
		--i;
	}
	return ERROR_RESEARCH_FAULT;
}

// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status) {
	if (start < 0 || start >= bmap->num_bits) return ERROR_RESEARCH_FAULT;
	int i = start;
	while (i < bmap->num_bits) {
		int pos = BitMap_check((bmap->entries)[i], status);
		if (pos != ERROR_RESEARCH_FAULT) return i + (7 - pos);
	}
	return ERROR_RESEARCH_FAULT;
}

// sets the bit at index pos in bmap to status
//FUNZIONA SOLO CON STATUS = 1
int BitMap_set(BitMap* bmap, int pos, int status) {
	if (pos < 0 || pos >= bmap->numbits) return ERROR_RESEARCH_FAULT;
	int array_index = pos / NUMBITS;
	int offset = pos % NUMBITS;
	(bmap->entries)[array_index] ^= (1 << offset);
}
