#include "simplefs.h"

// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk) {

	fs->disk = disk;
	
	// Creating the Directory Handle and filling it
	DirectoryHandle* handle  = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
	FirstDirectoryBlock* firstdir = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
	
	// If operating on a new disk return NULL: we need to format it.
	int snorlax = DiskDriver_readBlock(disk, firstdir, 0);
	if (snorlax) return NULL;
	
	// Filling the handle
	handle->sfs = fs;
	handle->dcb = firstdir;
	handle->directory = NULL;
	handle->current_block = (&firstdir->header);
	handle->pos_in_dir = 0;
	handle->pos_in_block = 0;
	
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
	header.block_in_disk = 0;

	// Creating the FCB
	FileControlBlock fcb;
	fcb.directory_block = 0;
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
		firstdir.file_blocks[i] = TBA;
	}
	
	DiskDriver_writeBlock(fs->disk, &firstdir, 0); 
	
}

// Aux for SimpleFS_createFile.
// Returns the header of the directory block in where to store the new file
// creating the block if necessary.
// Return NULL if the file already exists.
BlockHeader* SimpleFS_lennyfoo(DirectoryHandle* d, const char* filename) {
	
	// Setting and checking for DiskDriver
	DiskDriver* disk = d->sfs->disk;
	if (disk == NULL) return NULL;
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	BlockHeader* header = &d->dcb->header;
	FirstFileBlock* file = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	int snorlax = TBA;
	int filecount = 0;
	// searching in the first directory block
	// if there is a file with the same name
	
	d->pos_in_dir = 0;
	d->pos_in_block = 0;
	while (d->pos_in_block < fbsize) {
		if (d->dcb->file_blocks[d->pos_in_block] != TBA) {
			++filecount;
			snorlax = DiskDriver_readBlock(disk, file, d->dcb->file_blocks[d->pos_in_block]);
			if (snorlax == 0 && file->fcb.is_dir == FIL) {
				if (strcmp(file->fcb.name, filename) == 0) {
					printf ("ALREADY EXISTS A FILE WITH THE SAME NAME! lennyfoo 1\n");
					free(dirblock);
					return NULL;
				}
			}
		}
		++d->pos_in_block;		
	}
	
	// if the directory is not full there's no need of creating other blocks
	// if false, at the end of these instructions the header should be the same of the dirhandle's
	// if true, the header is still the same
	if (filecount != d->pos_in_block) {
		d->current_block = header;
		d->pos_in_dir = header->block_in_file;
		d->dcb->num_entries = filecount;
		free(dirblock);
		return header;
	}
	// if the directory is full and is composed by multiple blocks
	// searching in the next blocks
	// if enters in the while, at the end of these instructons the header shoult be last visited dirblock's header
	// if doesn't enter in the while, the header should be the same of the dirhandle's
	while (header->next_block != TBA) {
		++d->pos_in_dir;
		header->block_in_file = d->pos_in_dir;
		d->pos_in_block = 0;
		snorlax = DiskDriver_readBlock(disk, dirblock, header->next_block);
		if (snorlax == TBA) return NULL;
		while (d->pos_in_block < bsize) {
			if (dirblock->file_blocks[d->pos_in_block] != TBA) {
				++filecount;
				snorlax = DiskDriver_readBlock(disk, file, dirblock->file_blocks[d->pos_in_block]);
				if (snorlax == 0 && file->fcb.is_dir == FIL) {
					if (strcmp(file->fcb.name, filename) == 0) {
						printf ("ALREADY EXISTS A FILE WITH THE SAME NAME!	lennyfoo 2\n");
						return NULL;
					}
				}
			}
			++d->pos_in_block;
		}
		header = &dirblock->header;
	}
	
	// if the directory is not full there's no need of creating other blocks
	// updating dirhandle informations on the last block visited
	if (filecount != fbsize + (bsize * d->pos_in_dir)) {
		d->current_block = header;
		d->dcb->num_entries = filecount;
		return header;
	}
	
	// if the function arrives at this point means that
	// there's need to create a new dirblock and attach it at the actual header
	
	snorlax = DiskDriver_getFreeBlock(disk, 0);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY READING SOMETHING\n");
		return NULL;
	}

	memset(dirblock, 0, sizeof(DirectoryBlock));
		
	dirblock->header.previous_block = header->block_in_disk;
	dirblock->header.next_block = TBA;
	dirblock->header.block_in_file = header->block_in_file + 1;
	dirblock->header.block_in_disk = snorlax;
	for (int j = 0; j < bsize; ++j) {
		dirblock->file_blocks[j] = TBA;
	}
	
	// at this point, dirblocks only lives in the function.
	// writing it into the disk should definitively create it.
	
	// writing the new block on the disk
	snorlax = DiskDriver_writeBlock(disk, dirblock, dirblock->header.block_in_disk);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY WRITING SOMETHING\n");
		return NULL;
	}	

	printf ("AAAAAAAAAAAAAAAAAAAA HODOOOOOOOOOOOOOOOOOOOOR %d          lennyfoo\n", dirblock->header.block_in_disk);
	// updating the fcb
	d->dcb->header.next_block = dirblock->header.block_in_disk;
	d->dcb->fcb.size_in_bytes += BLOCK_SIZE;
	d->dcb->fcb.size_in_blocks += 1;
	snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->fcb.block_in_disk);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY WRITING SOMETHING\n");
		return NULL;
	}
	
	// updating dirhandle
	d->current_block = &dirblock->header;
	d->pos_in_dir = dirblock->header.block_in_file;
	
	return &dirblock->header;
 
}

// Gets the first free position in a directory block array 
// to avoid to fill a directory with deleted files
int SimpleFS_get13pos (DirectoryHandle* d, BlockHeader* header) {
	
	void* aux_block = (void*) malloc(BLOCK_SIZE);
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	if (header->block_in_disk == 0) {
		FirstDirectoryBlock* fdb = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
		int snorlax = DiskDriver_readBlock(d->sfs->disk, fdb, header->block_in_disk);
		if (snorlax == TBA) return TBA;
		for (int i = 0; i < fbsize; ++i) {
			if (fdb->file_blocks[i] == TBA) return i;
			snorlax = DiskDriver_readBlock(d->sfs->disk, aux_block, fdb->file_blocks[i]);			
			if (snorlax == TBA) return i;
		}
	}
	if (header->block_in_disk != 0) {
		DirectoryBlock* db = (DirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
		int snorlax = DiskDriver_readBlock(d->sfs->disk, db, header->block_in_disk);
		if (snorlax == TBA) return TBA;
		for (int i = 0; i < bsize; ++i) {
			if (db->file_blocks[i] == TBA) return i;
			snorlax = DiskDriver_readBlock(d->sfs->disk, aux_block, db->file_blocks[i]);
			if (snorlax == TBA) return i;
		}
	}
	return TBA;

}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename) {	
	
	// Checking for DirectoryHandle existance
	if (d == NULL) {
		return NULL;
	}
	
	// Setting and checking for DiskDriver
	DiskDriver* disk = d->sfs->disk;
	if (disk == NULL) return NULL;
	
	// Checking for remaining free blocks
	// Might need 2 free block to store the file block and an additional directory block
	if (disk->header->free_blocks <= 1) return NULL;
	
	// Creating useful things
	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	FirstFileBlock* file = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));

	// Getting the header of the directory block in where to store the file
	BlockHeader* header = SimpleFS_lennyfoo(d, filename);
	if (header == NULL) return NULL;
	
	// Resetting FirstFileBlock* file to fill it with the right block
	// Voyager is the block_num of the file in the blocklist	
	int voyager = DiskDriver_getFreeBlock(disk, 0);
	if (voyager == ERROR_FS_FAULT) {
		printf ("ERROR : NO FREE BLOCKS AVAILABLE\n");
		return NULL;
	}
	int snorlax = DiskDriver_readBlock(disk, file, voyager);
	if (!snorlax) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
		return NULL;
	}
	
	memset(file, 0, sizeof(FirstFileBlock));
		
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
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
		return NULL;
	}
	

	// if header->block_in_file == 0
	// the header is d->dcb's header
	// so must update this on the disk
	// else is necessary to a write the corresponding header's block 
	// (read it, update it, rewrite it)
	
	// getting the first free index in where to store the file
	int hodor = SimpleFS_get13pos(d, header);
	if (header->block_in_file == 0) {
		
		// Updating refs in DirectoryHandle's directory d->dcb
		d->dcb->file_blocks[hodor] = voyager;
		++d->dcb->num_entries;

		// Updating the directory in disk, too
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->fcb.block_in_disk);
		if (snorlax == ERROR_FS_FAULT) {
			printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
			return NULL;
		}
	} else {
		
		DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
		snorlax = DiskDriver_readBlock(disk, dirblock, header->block_in_disk);
		if (snorlax == ERROR_FS_FAULT) {
			printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
			return NULL;
		}
		dirblock->file_blocks[hodor] = voyager;
		++d->dcb->num_entries;
		snorlax = DiskDriver_writeBlock(disk, dirblock, header->block_in_disk);
		
		
	}
	
	// Filling the handle
	handle->sfs = d->sfs;
	handle->fcb = file;
	handle->directory = d->dcb;
	handle->current_block = &handle->fcb->header;
	handle->pos_in_file = 0;
	
	return handle;
}

// reads in the (preallocated) blocks array, the name of all files in a directory 
int SimpleFS_readDir(char** names, DirectoryHandle* d) {	
	int len = d->dcb->num_entries;
	int i = 0;
	FirstFileBlock f;
	
	while (i < len) {		
		DiskDriver_readBlock(d->sfs->disk, &f, d->dcb->file_blocks[i]);
		strcpy(names[i], f.fcb.name);
		++i;
	}

	return i;
}


// opens a file in the  directory d. The file should be exisiting
// returns NULL if the file does not exist
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename) {

	// Checking for DirectoryHandle existance
	if (d == NULL) {
		return NULL;
	}
	
	DiskDriver* disk = d->sfs->disk;
	
	// Creating useful things
	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	FirstFileBlock* file = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	
	// Checking for existing file	
	// Scanning all blocks in file_blocks: 
	// IF the block is occupied in the bitmap (voyager = 0)
	// && IF the scanned file's name == filename
	// MEANS THAT the searched file has ben found
	// SO we can create the handle
	for (int i = 0; i < d->dcb->num_entries; ++i) {
		int voyager = DiskDriver_readBlock(disk, file, d->dcb->file_blocks[i]);
		if (voyager == 0) {		
			if (strcmp(file->fcb.name, filename) == 0) {
				
				handle->sfs = d->sfs;
				handle->fcb = file;
				handle->directory = d->dcb;
				handle->current_block = &file->header;
				handle->pos_in_file = 0;
				
				return handle;
			}
		}		
	}
		
	return NULL;
	
}


// closes a file handle (destroyes it)
// RETURNS 0 on success, -1 if fails
int SimpleFS_close(FileHandle* f) {

	if (f == NULL) return -1;
	free(f->fcb);
	free(f);
	f = NULL;

	return 0; 
}

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size);

// reads in the file, at current position size bytes stored in data
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
