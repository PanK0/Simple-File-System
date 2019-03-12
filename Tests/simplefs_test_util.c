#include "simplefs_test_util.h"
#include <stdio.h>

// Prints a FirstDirectoryBlock
void SimpleFS_printFirstDir(SimpleFS* fs, FirstDirectoryBlock* d) {
	if (DiskDriver_readBlock(fs->disk, d, 0) == 0) {
		printf ("-- Root Directory \n");
		printf ("Name            : %s\n", d->fcb.name);
		printf ("Previous Block  : %d\n", d->header.previous_block);
		printf ("Next Block      : %d\n", d->header.next_block); 
		printf ("Directory Block : %d\n", d->fcb.directory_block);
		printf ("Size in bytes   : %d\n", d->fcb.size_in_bytes);
		printf ("Size in blocks  : %d\n", d->fcb.size_in_blocks);
	}
	else {
		printf ("FILE SYSTEM NOT INITIALIZED YET\n");
	}
}

// Prints the Disk Driver content
void SimpleFS_print (SimpleFS* fs) {
	printf ("-------- DISK DRIVER --------\n");
	DiskDriver* disk = fs->disk;
	printf ("Header\n");
	printf ("num_blocks	        : %d\n", disk->header->num_blocks);
	printf ("bitmap_blocks		: %d\n", disk->header->bitmap_blocks);
	printf ("bitmap_entries		: %d\n", disk->header->bitmap_entries);
	printf ("free_blocks		: %d\n", disk->header->free_blocks);
	printf ("first_free_block	: %d\n", disk->header->first_free_block);
	
	for (int i = 0; i < disk->header->bitmap_entries; ++i) {
		printf ("[ %d ] ", disk->bitmap_data[i]);
	}
	
	printf ("\n\n-------- FILE SYSTEM --------\n");
	FirstDirectoryBlock firstdir;
	SimpleFS_printFirstDir(fs, &firstdir);
	
	
	printf ("\n");
}
