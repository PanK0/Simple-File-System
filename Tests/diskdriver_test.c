#include "diskdriver_test_util.c"

#define NUM_BLOCKS 51

int main (int argv, char** argc) {
	
	// Initializing the disk - testing DiskDriver_init()
	printf ("**  Inizializing the disk - testing DiskDriver_init()\n");
	
	DiskDriver disk;
	DiskDriver_init(&disk, "disk_test.txt", NUM_BLOCKS);
	DiskDriver_print(&disk);
	
	
	// Writing on the disk - testing DiskDriver_writeBlock()
	printf ("\n\n**  Writing on the disk - testing DiskDriver_writeBlock()\n");
	char src[BLOCK_SIZE];
	
	for (int i = 0; i < BLOCK_SIZE; ++i) {
		src[i] = '@';
	}	
	
	
	for (int i = 0; i < NUM_BLOCKS; ++i) {
		if (i%2) DiskDriver_writeBlock(&disk, (void*)src, i);
	}	
	
	DiskDriver_writeBlock(&disk, (void*)src, 0);
	DiskDriver_writeBlock(&disk, (void*)src, NUM_BLOCKS/2);
	DiskDriver_print(&disk);
	
	
	// Reading the disk - testing DiskDriver_readBlock()
	// returns -1 if the block is free, 0 otherwise
	printf ("\n\n**  Reading the disk - testing DiskDriver_readBlock()\n");
	char dest [BLOCK_SIZE];
	
	int voyager;
	voyager = DiskDriver_readBlock(&disk, (void*)dest, NUM_BLOCKS/2);
	printf ("The read gives : %d\n", voyager);
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, 21);
	printf ("The read gives : %d\n", voyager);
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, 22);
	printf ("The read gives : %d\n", voyager);
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, 0);
	printf ("The read gives : %d\n", voyager);
	
	
	
	// Freeing a block - testing DiskDriver_freeBlock()
	printf ("\n\n**  Freeing a block - testing DiskDriver_freeBlock()\n");
	voyager = DiskDriver_freeBlock(&disk, NUM_BLOCKS/2);
	printf ("Blcok number %d should be free now\n", NUM_BLOCKS/2);
	DiskDriver_print(&disk);
	
	
	for (int i = 0; i < NUM_BLOCKS; ++i) {
		DiskDriver_freeBlock(&disk, i);
	}
	
	printf ("\n\n**  Freeing all blocks - all should be 0\n");
	DiskDriver_print(&disk);
	
	DiskDriver_flush(&disk);
	
	DiskDriver_unmap(&disk);
	close(disk.fd);
	return 0;
}
