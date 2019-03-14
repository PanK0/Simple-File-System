#include "simplefs.h"

// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk) {

	fs->disk = disk;
	
	// Creating the Directory Handle and filling it
	DirectoryHandle* handle  = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
	
	//off_t blocklist_start = sizeof(fs->disk->header) + fs->disk->header->bitmap_entries;
	//FirstDirectoryBlock* firstdir = (FirstDirectoryBlock*) fs->disk->header + blocklist_start;
	FirstDirectoryBlock* firstdir = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
	
	//void* block = malloc(BLOCK_SIZE);
	
	// If operating on a new disk return NULL: we need to format it.
	int snorlax = DiskDriver_readBlock(disk, firstdir, 0);
	if (snorlax) return NULL;
	
	// Filling the handle
	handle->sfs = fs;
	handle->dcb = firstdir;
	handle->directory = NULL;
	handle->current_block = &firstdir->header;
	handle->pos_in_dir = 0;
	handle->pos_in_block = 0;
	
	printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d\n", handle->sfs->disk->header->free_blocks);
	printf ("############################Directory             : %s\n", handle->dcb->fcb.name);
	printf ("############################Is Dir?               : %d\n", handle->dcb->fcb.is_dir);
	printf ("############################Block in disk         : %d\n", handle->dcb->fcb.block_in_disk);
	
	return handle;	
}

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs) {
	
	// First of all, free the disk
	int num_blocks = fs->disk->header->num_blocks;
	for (int i = 0; i < num_blocks; ++i) {
		DiskDriver_freeBlock(fs->disk, i);
	}
	
	// Once the disk is free, creating the Directory Header
	BlockHeader header;
	header.previous_block = TBA;
	header.next_block = TBA;
	header.block_in_file = 0;

	// Creating the FCB
	FileControlBlock fcb;
	fcb.directory_block = TBA;
	fcb.block_in_disk = 0;
	strcpy(fcb.name, "/");
	fcb.size_in_bytes = sizeof(FirstDirectoryBlock);
	fcb.size_in_blocks = 1;
	fcb.is_dir = DIR;
		
	// Creating the First Directory Block
	FirstDirectoryBlock firstdir;
	firstdir.header = header;
	firstdir.fcb = fcb;
	firstdir.num_entries = 0;
	int size = (BLOCK_SIZE - sizeof(BlockHeader) - sizeof(FileControlBlock) - sizeof(int)) / sizeof(int);
	for (int i = 0; i < size; ++i) {
		firstdir.file_blocks[i] = 0;
	}
	
	DiskDriver_writeBlock(fs->disk, &firstdir, 0); 
	
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename) {
	
	// Checking for DirectoryHandle existance
	if (d == NULL) {
		return NULL;
	}
	
	DiskDriver* disk = d->sfs->disk;
	
	// Checking for remaining free blocks
	if (disk->header->free_blocks == 0) return NULL;
	
	// Creating useful things
	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	FirstFileBlock* file = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	
	// Checking for existing file
	printf ("###############\n");
	for (int i = 0; i < d->dcb->num_entries; ++i) {
		printf (" %d ", d->dcb->file_blocks[i]); 
	}
	printf ("\n");
	
	for (int i = 0; i < d->dcb->num_entries; ++i) {
		int voyager = DiskDriver_readBlock(disk, (void*)file, d->dcb->file_blocks[i]);
		if (voyager == -1) {	
			if (strcmp(file->fcb.name, filename) == 0) {
				printf ("ALREADY EXISTS A FILE WITH THE SAME NAME!\n");
				return NULL;
			}
		}
	}
	
	// Resetting FirstFileBlock* file to fill it with the right block
	// Voyager is the block_num of the file in the blocklist
	memset(file, 0, sizeof(FirstFileBlock));
	int voyager = DiskDriver_getFreeBlock(disk, 0);
	if (voyager == ERROR_NO_FREE_BLOCKS) {
		printf ("ERROR : NO FREE BLOCKS AVAILABLE\n CLOSING . . .");
		return NULL;
	}
	int snorlax = DiskDriver_readBlock(disk, (void*)file, voyager);
	if (!snorlax) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n CLOSING . . .");
		return NULL;
	} 
		
	// Filling the FirstFileBlock with the right structures
	file->header.previous_block = TBA;
	file->header.next_block = TBA;
	file->header.block_in_file = 0;

	file->fcb.directory_block = d->dcb->header.block_in_file;
	file->fcb.block_in_disk = voyager;
	strcpy(file->fcb.name, filename);
	file->fcb.size_in_bytes = sizeof(FirstFileBlock);
	file->fcb.size_in_blocks = 1;
	file->fcb.is_dir = FIL;
	strcpy(file->data, "\0");
	
	// Writing the file on the disk
	snorlax = DiskDriver_writeBlock(disk, file, voyager);
	if (snorlax == ERROR_FILE_WRITING) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n CLOSING . . .");
		return NULL;
	}
	
	// Updating refs in d->dcb
	d->dcb->file_blocks[d->dcb->num_entries] = voyager;
	printf ("AAAAAAAAAAAAAAAAAAAA %d\n", d->dcb->num_entries);
	++d->dcb->num_entries;
	printf ("AAAAAAAAAAAAAAAAAAAA %d\n", d->dcb->num_entries);
	
	// Filling the handle
	handle->sfs = d->sfs;
	handle->fcb = file;
	handle->directory = d->dcb;
	handle->current_block = &handle->fcb->header;
	handle->pos_in_file = 0;
	
	return handle;
}

// reads in the (preallocated) blocks array, the name of all files in a directory 
int SimpleFS_readDir(char** names, DirectoryHandle* d);


// opens a file in the  directory d. The file should be exisiting
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename);


// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f);

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size);

// writes in the file, at current position size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size);

// returns the number of bytes read (moving the current pointer to pos)
// returns pos on success
// -1 on error (file too short)
int SimpleFS_seek(FileHandle* f, int pos);

// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
 int SimpleFS_changeDir(DirectoryHandle* d, char* dirname);

// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname);

// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(SimpleFS* fs, char* filename);
