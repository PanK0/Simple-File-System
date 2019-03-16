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
	
	// Creating an already existent file
	printf ("\n**	Creating an already existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, "HELLO");
	SimpleFS_printHandle(filehandle);
	
	// Creating a non-existent file
	printf ("\n**	Creating a non-existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, "POt_aTO");
	SimpleFS_printHandle(filehandle);
	
	// Creating an non-existent file
	printf ("\n**	Creating a non-existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, "MUNNEZZ");
	SimpleFS_printHandle(filehandle);
	
	// Creating an already existent file
	printf ("\n**	Creating an already existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, "POt_aTO");
	SimpleFS_printHandle(filehandle);
	
	
	// Printing the updated File System
	printf ("\n");
	SimpleFS_print(&fs);
	
	// Reading a folder
	printf ("\n**	Reading all files in a directory - testing SimpleFS_readDir() \n");
	char* names[dirhandle->dcb->num_entries];

	
	FirstFileBlock f;
	DiskDriver_readBlock(dirhandle->sfs->disk, &f, dirhandle->dcb->file_blocks[0]);
	names[0] = f.fcb.name;
	printf ("SSSSSSSSSSSSSS %s, %s\n", f.fcb.name, names[0]);
	DiskDriver_readBlock(dirhandle->sfs->disk, &f, dirhandle->dcb->file_blocks[1]);
	names[1] = f.fcb.name;
	printf ("SSSSSSSSSSSSSS %s, %s\n", f.fcb.name, names[1]);
	DiskDriver_readBlock(dirhandle->sfs->disk, &f, dirhandle->dcb->file_blocks[2]);
	names[2] = f.fcb.name;
	printf ("SSSSSSSSSSSSSS %s, %s\n", f.fcb.name, names[2]);

	//SimpleFS_readDir(names, dirhandle);
	
	for (int i = 0; i < dirhandle->dcb->num_entries; i++) {
		printf ("%d %s ",i , names[i]);
	}
	
	DiskDriver_flush(&disk);	
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
