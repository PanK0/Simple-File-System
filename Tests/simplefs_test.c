#include "simplefs_test_util.c"

#define NUM_BLOCKS 	500
#define NUM_FILES	400
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
	FileHandle* filehandle;
	dirhandle = SimpleFS_init(&fs, &disk);
	SimpleFS_print(&fs, dirhandle);
	SimpleFS_printHandle((void*)dirhandle);
	
	if (dirhandle == NULL) {
		// Formatting the disk
		printf ("\n\n**	Formatting File System - testing SimpleFS_format()\n");
		SimpleFS_format(&fs);
		dirhandle = SimpleFS_init(&fs, &disk);
		SimpleFS_print(&fs, dirhandle);
	}
	
	// Giving current location
	printf ("\n");
	SimpleFS_printHandle(dirhandle);
	
	// Generating random strings and creating files
	// Using to test directory block capability limits
	printf ("\n\n-------- Creating a lot of random files in the root dir--------\n");
	char filenames[NUM_FILES][10];
	for (int i = 0; i < NUM_FILES; ++i) {
			gen_filename(filenames[i], i);
			//printf ("Blocco :%d, %s\n", dirhandle->current_block->block_in_file, filenames[i]);
			//printf ("%d ", i);
			filehandle = SimpleFS_createFile(dirhandle, filenames[i]);
	}
	printf ("\n");
	
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
*/	
	// Printing the updated File System
	printf ("\n");
	SimpleFS_print(&fs, dirhandle);
	
	printf ("\n");
	// Giving current location
	printf ("\n");
	SimpleFS_printHandle(dirhandle);

	// Printing the directory block list
	printf ("\n\n** PRINTING THE DIRECTORY BLOCK LIST \n");;
	SimpleFS_printDirBlocks(dirhandle);


	// Reading a folder
	// Allocating an array
	printf ("\n**	Reading all files in a directory - testing SimpleFS_readDir() \n");
	char* names[dirhandle->dcb->num_entries];
	for (int i = 0; i < dirhandle->dcb->num_entries; ++i) {
		names[i] = (char*) malloc(NAME_SIZE);
	}

	SimpleFS_readDir(names, dirhandle);
/*	
	printf ("\n**	Listing files in directory %s\n", dirhandle->dcb->fcb.name);
	SimpleFS_printArray(names, dirhandle->dcb->num_entries);
*/

	// Opening a file
	printf ("\n**	Opening a file - testing SimpleFS_openFile() \n");
	filehandle = SimpleFS_openFile(dirhandle, names[NUM_FILES/2]);
	SimpleFS_printHandle(filehandle);

/*	
	// Closing a file - DOES NOT WORK
	printf ("\n**	Closing a file - testing SimpleFS_close() \n");
	SimpleFS_close(filehandle);
	SimpleFS_printHandle(filehandle);
*/

	// Writing a file
	printf ("\n**	Writing a file - testing SimpleFS_write() \n");
	char text[] = "Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura";
	int size = sizeof(text) / sizeof(char);
	int wdata = SimpleFS_write(filehandle, text, size);
	printf ("Written : %d bytes. Should be : %d\n", wdata, size);
	
	DiskDriver_flush(&disk);
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
