#include "../FS/bitmap.c"

typedef struct ListItem {
	BitMapEntryKey entry;
	struct ListItem* prev;
	struct ListItem* next;
} ListItem;

typedef struct ListHead {
	ListItem* first;
	ListItem* last;
	int size;
} ListHead;

static void List_init(ListHead* head);
static void BitMapEntryKey_print(BitMapEntryKey* entry);
static void BitMap_create(BitMap* bmap, int size, uint8_t* entries);
static void BitMap_print(BitMap* bmap);
static ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
static ListItem* List_detach(ListHead* head, ListItem* item);
