#include "diskdriver_test_util.c"

#define NUM_BLOCKS 71

int main (int argv, char** argc) {
	
	// Initializing the disk
	printf ("**  Inizializing the disk - testing DiskDriver_init()\n");
	
	DiskDriver disk;
	DiskDriver_init(&disk, "disk_test", NUM_BLOCKS);
	
	
	return 0;
}
