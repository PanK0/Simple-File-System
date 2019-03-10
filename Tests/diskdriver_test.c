#include "diskdriver_test_util.c"

#define NUM_BLOCKS 53

int main (int argv, char** argc) {
	
	// Initializing the disk - testing DiskDriver_init()
	printf ("**  Inizializing the disk - testing DiskDriver_init()\n");
	
	DiskDriver disk;
	DiskDriver_init(&disk, "disk_test.txt", NUM_BLOCKS);
	DiskDriver_print(&disk);
	
	
	// Writing on the disk - testing DiskDriver_writeBlock()
	printf ("\n\n**  Writing on the disk - testing DiskDriver_writeBlock()\n");
	char* src = "Pulvis es et pulveris revertabiris\n";
	DiskDriver_writeBlock(&disk, (void*)src, 0);
	DiskDriver_writeBlock(&disk, (void*)src, NUM_BLOCKS/2);
	DiskDriver_print(&disk);
	
	
	// Reading the disk - testing DiskDriver_readBlock()
	printf ("\n\n**  Reading the disk - testing DiskDriver_readBlock()\n");
	char dest [BLOCK_SIZE];
	int voyager;
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, NUM_BLOCKS/2);
	printf ("The read gives : %d, should give : 0\n", voyager);
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, 21);
	printf ("The read gives : %d, should give : -1\n", voyager);
	
	voyager = DiskDriver_readBlock(&disk, (void*)dest, 0);
	printf ("The read gives : %d, should give : 0\n", voyager);
	
	
	// Freeing a block - testing DiskDriver_freeBlock()
	printf ("\n\n**  Freeing a block - testing DiskDriver_freeBlock()\n");
	voyager = DiskDriver_freeBlock(&disk, NUM_BLOCKS/2);
	printf ("Blcok number %d should be free now\n", NUM_BLOCKS/2);
	DiskDriver_print(&disk);
	
	
	close(disk.fd);
	return 0;
}
