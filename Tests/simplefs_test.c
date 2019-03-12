#include "simplefs_test_util.c"

#define NUM_BLOCKS 51

int main (int argc, char** argv) {
	
	DiskDriver disk;
	DiskDriver_init(&disk, "simplefs_test.txt", NUM_BLOCKS);
	
	SimpleFS fs;
	SimpleFS_init(&fs, &disk);
	
	SimpleFS_print(&fs);
	
	SimpleFS_format(&fs);
	printf ("HELLO WORLD\n");
	
	return 0;
}
