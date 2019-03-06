#include "bitmap_test_util.h"
#include <stdio.h>

static void List_init(ListHead* head) {
	head->first=0;
	head->last=0;
	head->size=0;
}

static void BitMapEntryKey_print(BitMapEntryKey* entry) {
	printf ("[ entry_num: %d, ", entry->entry_num);
	printf ("array cell: %d, ", (entry->entry_num) / NUMBITS);
	printf ("bit_num: %d ]\n", entry->bit_num);
	
}

static void BitMap_create(BitMap* bmap, int size, uint8_t* entries) {
	bmap->num_bits = size;
	bmap->entries = entries;
	for (int i = 0; i < size; i++) {
		bmap->entries[i] = 0;
	}	
}


static void BitMap_print(BitMap* bmap) {
	printf ("Size: %d\n", bmap->num_bits);
	printf ("[ ");
	for (int i = 0; i < 512; i++) {
		printf("%d ", bmap->entries[i]);
	}
	printf ("]\n");
}


static ListItem* List_insert(ListHead* head, ListItem* prev, ListItem* item) {
	if (item->next || item->prev)
		return 0;
	ListItem* next= prev ? prev->next : head->first;
	if (prev) {
		item->prev = prev;
		prev->next = item;
	}
	if (next) {
		item->next = next;
		next->prev = item;
	}
	if (!prev)
		head->first = item;
	if(!next)
		head->last = item;
	++head->size;
	return item;
}

static ListItem* List_detach(ListHead* head, ListItem* item) {
	ListItem* prev = item->prev;
	ListItem* next = item->next;
	if (prev){
		prev->next = next;
	}
	if(next){
		next->prev = prev;
	}
	if (item == head->first)
		head->first = next;
	if (item == head->last)
		head->last = prev;
	head->size--;
	item->next = item->prev = 0;
	return item;
}
