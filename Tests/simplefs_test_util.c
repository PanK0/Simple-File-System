#include "simplefs_test_util.h"
#include <stdio.h>

// prints the Disk Driver content
void SimpleFS_print (SimpleFS* fs) {
	DiskDriver* disk = fs->disk;
	printf ("Header\n");
	printf ("num_blocks 		: %d\n", disk->header->num_blocks);
	printf ("bitmap_blocks		: %d\n", disk->header->bitmap_blocks);
	printf ("bitmap_entries		: %d\n", disk->header->bitmap_entries);
	printf ("free_blocks		: %d\n", disk->header->free_blocks);
	printf ("first_free_block	: %d\n", disk->header->first_free_block);
	
	for (int i = 0; i < disk->header->bitmap_entries; ++i) {
		printf ("[ %d ] ", disk->bitmap_data[i]);
	}
	printf ("\n");
}
