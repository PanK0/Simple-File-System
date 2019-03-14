#include "simplefs_test_util.c"

#define NUM_BLOCKS 51

int main (int argc, char** argv) {
	
	// Init the disk and the file system
	printf ("**	Initializing Disk and File System - testing SimpleFS_init()\n");
	
	DiskDriver disk;
	DiskDriver_init(&disk, "simplefs_test.txt", NUM_BLOCKS);
	
	SimpleFS fs;
	DirectoryHandle* dirhandle;
	dirhandle = SimpleFS_init(&fs, &disk);
	SimpleFS_print(&fs);
	
	if (dirhandle == NULL) {
		// Formatting the disk
		printf ("\n\n**	Formatting File System - testing SimpleFS_format()\n");
		SimpleFS_format(&fs);
		dirhandle = SimpleFS_init(&fs, &disk);
		SimpleFS_print(&fs);
	}
	
	// Giving current location
	printf ("\n");
	SimpleFS_printHandle(dirhandle);

	// Creating a file
	printf ("\n**	Creating a file - testing SimpleFs_createFile() \n");
	FileHandle* filehandle;
	filehandle = SimpleFS_createFile(dirhandle, "HELLO");
	SimpleFS_printHandle(filehandle);
/*	
	// Creating an already existent file
	printf ("\n**	Creating an already existent file - testing SimpleFs_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, "HELLO");
	SimpleFS_printHandle(filehandle);
*/	
	
	DiskDriver_flush(&disk);	
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
