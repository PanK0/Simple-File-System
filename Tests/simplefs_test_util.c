#include "simplefs_test_util.h"
#include <stdio.h>

// Prints a FirstDirectoryBlock
void SimpleFS_printFirstDir(SimpleFS* fs, DirectoryHandle* d) {
	if (DiskDriver_readBlock(fs->disk, d, 0) == 0) {
		printf ("-- Root Directory  --- SimpleFS_printFirstDir()\n");
		printf ("Name            : %s\n", d->dcb->fcb.name);
		printf ("Previous Block  : %d\n", d->dcb->header.previous_block);
		printf ("Next Block      : %d\n", d->dcb->header.next_block); 
		printf ("Directory Block : %d\n", d->dcb->fcb.directory_block);
		printf ("Size in bytes   : %d\n", d->dcb->fcb.size_in_bytes);
		printf ("Size in blocks  : %d\n", d->dcb->fcb.size_in_blocks);
		printf ("Num Entries     : %d\n", d->dcb->num_entries);
		
	}
	else {
		printf ("FILE SYSTEM NOT INITIALIZED YET\n");
	}
}

// Prints the Disk Driver content
void SimpleFS_print (SimpleFS* fs, DirectoryHandle* d) {
	printf ("-------- DISK DRIVER --------    SimpleFS_print()\n");
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
	printf ("\n\n");
}

// Prints the current directory location
void SimpleFS_printHandle (void* h) {
	printf ("------- SimpleFS_printHandle() \n");
	if (h == NULL) {
		printf ("GIVEN HANDLER IS NULL\n");
		return;
	}
	if (((DirectoryHandle*) h)->dcb->fcb.is_dir == DIR) {
		DirectoryHandle* handle = (DirectoryHandle*) h;
		printf ("-- You are now working in \n");
		printf ("Directory             : %s\n", handle->dcb->fcb.name);
		printf ("Size in Blocks        : %d\n", handle->dcb->fcb.size_in_blocks);
		printf ("Size in Bytes         : %d\n", handle->dcb->fcb.size_in_bytes);
		printf ("Is Dir?               : %d\n", handle->dcb->fcb.is_dir);
		printf ("Block in disk         : %d\n", handle->current_block->block_in_disk);
		printf ("Block in file         : %d\n", handle->current_block->block_in_file);
		if (handle->directory != NULL) 
			printf ("This dir's parent is  : %s\n", handle->directory->fcb.name);
		else printf ("This dir is root\n");
		printf ("Files in this folder  : %d\n", handle->dcb->num_entries);
		printf ("Position in this dir  : %d (should be the same of Block in File)\n", handle->pos_in_dir);
		printf ("Previous block        : %d\n", handle->current_block->previous_block);
		printf ("Next block            : %d\n", handle->current_block->next_block);
		SimpleFS_printDirBlocks(handle);
	}
	if (((FileHandle*) h)->fcb->fcb.is_dir == FIL) {
		FileHandle* handle = (FileHandle*) h;
		printf ("-- You are now working in \n");
		printf ("File                : %s\n", handle->fcb->fcb.name);
		printf ("Size in Blocks        : %d\n", handle->fcb->fcb.size_in_blocks);
		printf ("Size in Bytes         : %d\n", handle->fcb->fcb.size_in_bytes);
		printf ("Is Dir?               : %d\n", handle->fcb->fcb.is_dir);
		printf ("Block in disk         : %d\n", handle->current_block->block_in_disk);
		printf ("Block in file         : %d\n", handle->current_block->block_in_file);
		printf ("Parent dir's block    : %d\n", handle->fcb->fcb.directory_block); 
		printf ("Pos in file           : %d\n", handle->pos_in_file);
		printf ("Previous block        : %d\n", handle->current_block->previous_block);
		printf ("Next block            : %d\n", handle->current_block->next_block);
		SimpleFS_printFileBlocks(handle);
	}
	
	printf ("\n");
}

// Prints an array of strings
void SimpleFS_printArray (char** a, int len) {
	printf ("[ ");
	for (int i = 0; i < len; ++i) {
		if (strcmp(a[i], "") != 0) printf ("%s ", a[i]);
	}
	printf ("]\n");
}

// Generates a random string
void gen_random(char *s, const int len) {
    static const char alphanum[] =     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

// Generates names for files
void gen_filename(char *s, int i) {
	sprintf(s, "File_%d", i);
}
