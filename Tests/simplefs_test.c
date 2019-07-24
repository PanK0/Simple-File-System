#include "simplefs_test_util.c"

#define NUM_BLOCKS 	150
#define NUM_FILES	100
#define FILE_0	"HELLO"
#define FILE_1	"POt_aTO"
#define FILE_2	"MUNNEZZ"
#define FILE_X	"cocumber"

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
	
	// Generating random strings and creating files
	// Using to test directory block capability limits
	printf ("\n\n-------- Creating 100 random files in the root dir--------\n");
	char filenames[100][7];
	for (int i = 0; i < NUM_FILES; ++i) {
			gen_random(filenames[i], 7);
			//printf ("Blocco :%d, file n %d\n", dirhandle->current_block->block_in_file, i);
			FileHandle* filehandle = SimpleFS_createFile(dirhandle, filenames[i]);
	}
	printf ("\n");
	// Giving current location
	printf ("\n");
	SimpleFS_printHandle(dirhandle);
	
	// Printing the updated File System
	printf ("\n");
	SimpleFS_print(&fs);
	
	/*
	// Creating a file
	printf ("\n**	Creating a file - testing SimpleFs_createFile() \n");
	FileHandle* filehandle;
	filehandle = SimpleFS_createFile(dirhandle, FILE_0);
	SimpleFS_printHandle(filehandle);
	
	// Creating a file
	printf ("\n**	Creating a file - testing SimpleFs_createFile() \n");;
	filehandle = SimpleFS_createFile(dirhandle, FILE_1);
	SimpleFS_printHandle(filehandle);
	
	// Creating a file
	printf ("\n**	Creating a file - testing SimpleFs_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_0);
	SimpleFS_printHandle(filehandle);
 
	// Creating an already existent file
	printf ("\n**	Creating an already existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_0);
	SimpleFS_printHandle(filehandle);
	
	// Creating a non-existent file
	printf ("\n**	Creating a non-existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_1);
	SimpleFS_printHandle(filehandle);
	
	// Creating an non-existent file
	printf ("\n**	Creating a non-existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_2);
	SimpleFS_printHandle(filehandle);
	
	// Creating an already existent file
	printf ("\n**	Creating an already existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_1);
	SimpleFS_printHandle(filehandle);
	
	
	// Printing the updated File System
	printf ("\n");
	SimpleFS_print(&fs);
	
	// Reading a folder
	// Allocating an array
	printf ("\n**	Reading all files in a directory - testing SimpleFS_readDir() \n");
	char* names[dirhandle->dcb->num_entries];
	for (int i = 0; i < dirhandle->dcb->num_entries; ++i) {
		names[i] = (char*) malloc(NAME_SIZE);
	}

	SimpleFS_readDir(names, dirhandle);
	SimpleFS_printHandle(dirhandle);
	printf ("\n**	Listing files in directory %s\n", dirhandle->dcb->fcb.name);
	SimpleFS_printArray(names, dirhandle->dcb->num_entries);
	
	
	// Opening files - testing SimpleFS_openFile() 
	FileHandle* fh;
	printf ("\n\n**	Opening a non-existent file - testing SimpleFS_openFile()\n");
	fh = SimpleFS_openFile(dirhandle, FILE_X);
	SimpleFS_printHandle(fh);
	
	printf ("\n\n**	Opening an already existent file - testing SimpleFS_openFile()\n");
	fh = SimpleFS_openFile(dirhandle, FILE_1);
	SimpleFS_printHandle(fh);
	
 THIS DOESN'T WORK	
	// Closing a file - testing Simple
	printf ("\n\n**	Closing an already existent file - testing SimpleFS_close()\n");
	int a = SimpleFS_close(fh);
	SimpleFS_printHandle(fh);
	printf ("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA %d\n", a);

	
	*/
	DiskDriver_flush(&disk);
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
