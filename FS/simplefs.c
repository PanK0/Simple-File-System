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
	handle->directory = firstdir;
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
BlockHeader* SimpleFS_createFile_aux(DirectoryHandle* d, const char* filename) {


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
	int pos_in_block = 0;
	
	// searching in the first directory block
	// if there is a file with the same name
	while (pos_in_block < fbsize) {
		if (d->dcb->file_blocks[pos_in_block] != TBA) {
			++filecount;
			snorlax = DiskDriver_readBlock(disk, file, d->dcb->file_blocks[pos_in_block]);
			if (snorlax == 0 && file->fcb.is_dir == FIL) {
				if (strcmp(file->fcb.name, filename) == 0) {
					//printf ("ALREADY EXISTS A FILE WITH THE SAME NAME!	lennyfoo 1\n");
					return NULL;
				}
			}
		}
		++pos_in_block;
	}
	
	// if the directory is not full there's no need of creating other blocks
	if (filecount != pos_in_block) {
		d->current_block = header;
		d->pos_in_dir = header->block_in_file;
		d->pos_in_block = filecount;
		
		return header;
	}
	
/*
	printf ("########_1 d->dcb->header, d->current_block, header \nprev_block: %d__%d__%d \nnext_block: %d__%d__%d \nblock_in_file: %d__%d__%d \nblock_in_disk: %d__%d__%d\n",
		d->dcb->header.previous_block, d->current_block->previous_block, header->previous_block,
		d->dcb->header.next_block, d->current_block->next_block, header->next_block,
		d->dcb->header.block_in_file, d->current_block->block_in_file, header->block_in_file,
		d->dcb->header.block_in_disk, d->current_block->block_in_disk, header->block_in_disk);
*/	
	
	// if filecount == pos_in_block the firstdirectoryblock is full.
	// must search in the next directory blocks (if there are)
	// current block should be equal to header now.
	while (header->next_block != TBA) {
		pos_in_block = 0;
		filecount = 0;
		snorlax = DiskDriver_readBlock(disk, dirblock, header->next_block);
		if (snorlax == TBA) return NULL;
		while (pos_in_block < bsize) {
			if (dirblock->file_blocks[pos_in_block] != TBA) {
				++filecount;
				snorlax = DiskDriver_readBlock(disk, file, dirblock->file_blocks[pos_in_block]);
				if (snorlax == 0 && file->fcb.is_dir == FIL) {
					if (strcmp(file->fcb.name, filename) == 0) {
						//printf ("ALREADY EXISTS A FILE WITH THE SAME NAME!	lennyfoo 2\n");
						return NULL;
					}
				}
			}
			++pos_in_block;
		}
		header = &dirblock->header;
	}
	
/*	
	printf ("@@@@@@@@_2 d->dcb->header, d->current_block, header, dirblock \nprev_block    : %d__%d__%d__%d \nnext_block    : %d__%d__%d__%d \nblock_in_file : %d__%d__%d__%d \nblock_in_disk : %d__%d__%d__%d\n",
		d->dcb->header.previous_block, d->current_block->previous_block, header->previous_block, dirblock->header.previous_block,
		d->dcb->header.next_block, d->current_block->next_block, header->next_block, dirblock->header.next_block,
		d->dcb->header.block_in_file, d->current_block->block_in_file, header->block_in_file, dirblock->header.block_in_file,
		d->dcb->header.block_in_disk, d->current_block->block_in_disk, header->block_in_disk, dirblock->header.block_in_disk);
*/		
	
	// if the directory is not full there's no need of creating other blocks
	if (filecount != pos_in_block) {
		d->current_block = header;
		d->pos_in_dir = header->block_in_file;
		d->pos_in_block = filecount;
		return header;
	}
	
	// if the function arrives at this point means that
	// there's need to create a new dirblock and attach it at the actual header
	
	snorlax = DiskDriver_getFreeBlock(disk, 0);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY READING SOMETHING\n");
		return NULL;
	}
	
	// setting the new dirblock
	dirblock->header.previous_block = header->block_in_disk;
	dirblock->header.next_block = TBA;
	dirblock->header.block_in_file = header->block_in_file + 1;
	dirblock->header.block_in_disk = snorlax;
	for (int j = 0; j < bsize; ++j) {
		dirblock->file_blocks[j] = TBA;
	}

	// writing the new block on the disk
	snorlax = DiskDriver_writeBlock(disk, dirblock, dirblock->header.block_in_disk);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY WRITING SOMETHING\n");
		return NULL;
	}	

	// updating current block and the header
	// if we are in the first directory block update it
	// else update just the current block
	
	// updating the fcb
	d->dcb->fcb.size_in_bytes += BLOCK_SIZE;
	d->dcb->fcb.size_in_blocks += 1;
		
	
	if (d->current_block->block_in_file == d->dcb->header.block_in_file) {
		d->dcb->header.next_block = dirblock->header.block_in_disk;	
	} 
	
	snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->fcb.block_in_disk);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY WRITING SOMETHING\n");
		return NULL;
	}
	 
	d->current_block->next_block = dirblock->header.block_in_disk;
	snorlax = DiskDriver_writeBlock(disk, d->current_block, d->current_block->block_in_disk);
	if (snorlax == ERROR_FS_FAULT) {
		printf ("ERROR : SNORLAX IS BLOCKING THE WAY WRITING SOMETHING\n");
		return NULL;
	}
	header->next_block = dirblock->header.block_in_disk;
	
/*	
	printf ("§§§§§§§§_3 d->dcb->header, d->current_block, header, dirblock \nprev_block    : %d__%d__%d__%d \nnext_block    : %d__%d__%d__%d \nblock_in_file : %d__%d__%d__%d \nblock_in_disk : %d__%d__%d__%d\n",
		d->dcb->header.previous_block, d->current_block->previous_block, header->previous_block, dirblock->header.previous_block,
		d->dcb->header.next_block, d->current_block->next_block, header->next_block, dirblock->header.next_block,
		d->dcb->header.block_in_file, d->current_block->block_in_file, header->block_in_file, dirblock->header.block_in_file,
		d->dcb->header.block_in_disk, d->current_block->block_in_disk, header->block_in_disk, dirblock->header.block_in_disk);
*/
	
	return &dirblock->header;

}

// Gets the first free position in a directory block array 
// to avoid to fill a directory with deleted files
// returns the number of the first free position,
// returns -1 if there are no free blocks
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
	BlockHeader* header = SimpleFS_createFile_aux(d, filename);
	if (header == NULL) return NULL;
	//printf ("######## prev block: %d\nnext block : %d\nblock in file : %d\nblock in disk : %d\n", header->previous_block, header->next_block, header->block_in_file, header->block_in_disk);
	
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
	file->header.block_in_disk = voyager;

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
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->fcb.block_in_disk);
		if (snorlax == ERROR_FS_FAULT) {
			printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
			return NULL;
		}
		snorlax = DiskDriver_writeBlock(disk, dirblock, header->block_in_disk);	
		if (snorlax == ERROR_FS_FAULT) {
			printf ("ERROR : SNORLAX IS BLOCKING THE WAY\n");
			return NULL;
		}	
	}
	
	// Filling the handle
	handle->sfs = d->sfs;
	handle->fcb = file;
	handle->directory = d->dcb;
	handle->current_block = &handle->fcb->header;
	handle->pos_in_file = 0;
	
	return handle;
}


// prints all blocks of a directory
void SimpleFS_printDirBlocks (DirectoryHandle* dirhandle) {
	printf ("-------- SimpleFs_printDirblocks() in simplefs.c \n");
	BlockHeader* iterator = (BlockHeader*)malloc(sizeof(BlockHeader));
	DirectoryBlock* dirblock = (DirectoryBlock*)malloc(sizeof(DirectoryBlock));
	
	printf ("\nFolder %s is composed by blocks : ", dirhandle->dcb->fcb.name);
	printf ("{ ");
	iterator = &dirblock->header;
	while (iterator->next_block != TBA) {
		printf ("%d ", iterator->next_block);
		int snorlax = DiskDriver_readBlock(dirhandle->sfs->disk, dirblock, iterator->next_block);
		if (snorlax == TBA) return;
		iterator = &dirblock->header;
	}
	printf (" }\n");

}


// reads in the (preallocated) blocks array, the name of all files in a directory 
int SimpleFS_readDir(char** names, DirectoryHandle* d) {	
	
	int blocklist_len = d->dcb->fcb.size_in_blocks;
	int blocklist_array[blocklist_len];
	BlockHeader* iterator = (BlockHeader*) malloc(sizeof(BlockHeader));
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	iterator = &dirblock->header;
	int i = 0;
	
	//printf ("AAAAAAAA i : %d, blocklist_len : %d\n", i, blocklist_len);
	
	while (iterator->next_block != TBA) {
		blocklist_array[i] = iterator->next_block;
		int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, iterator->next_block);
		if (snorlax == TBA) return snorlax;
		
		++i;
		iterator = &dirblock->header;
	}
/*	
	for (int g = 0; g < blocklist_len; ++g) {
		printf ("%d ", blocklist_array[g]);
	}
	printf ("\n");
*/	
	int j = 0;
	FirstFileBlock f;
	while (j < fbsize) {
		DiskDriver_readBlock(d->sfs->disk, &f, d->dcb->file_blocks[j]);
		strcpy(names[j], f.fcb.name);
		++j;
	}
	
	i = 1;
	//printf ("BBBBBBB i : %d, blocklist_len : %d\n", i, blocklist_len);
	while (i < blocklist_len) {
		int k = 0;
		int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, blocklist_array[i]);
		if (snorlax == TBA) return snorlax;
		while (k < bsize) {
			if (dirblock->file_blocks[k] == TBA) break;
			snorlax = DiskDriver_readBlock(d->sfs->disk, &f, dirblock->file_blocks[k]);
			if (snorlax == TBA) return snorlax;
			if (f.fcb.is_dir != FIL) break;
			strcpy(names[j], f.fcb.name);
			++k;
			++j;
		}
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
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	// Creating useful things
	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	BlockHeader* iterator = (BlockHeader*) malloc(sizeof(BlockHeader));
	FirstFileBlock* file = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	
	// Creating an array of all Directory Blocks
	int blocklist_len = d->dcb->fcb.size_in_blocks;
	int blocklist_array[blocklist_len];
	iterator = &dirblock->header;
	int i = 0;
	while (iterator->next_block != TBA) {
		blocklist_array[i] = iterator->next_block;
		int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, iterator->next_block);
		if (snorlax == TBA) return NULL;
		
		++i;
		iterator = &dirblock->header;
	}
	
	// Checking for existing file	
	// Scanning all blocks:
	// IF the scanned file's name == filename
	// MEANS THAT the searched file has been found
	// SO we can create the handle
	
	int j = 0;
	while (j < fbsize) {
		DiskDriver_readBlock(disk, file, d->dcb->file_blocks[j]);
		if (strcmp(filename, file->fcb.name) == 0) {
			
			handle->sfs = d->sfs;
			handle->fcb = file;
			handle->directory = d->dcb;
			handle->current_block = &file->header;
			handle->pos_in_file = 0;
			
			return handle;
		}
		++j;
	}
	
	i = 1;
	while (i < blocklist_len) {
		int k = 0;
		int snorlax = DiskDriver_readBlock(disk, dirblock, blocklist_array[i]);
		if (snorlax == TBA) return NULL;
		while (k < bsize) {
			if (dirblock->file_blocks[k] == TBA) break;
			snorlax = DiskDriver_readBlock(disk, file, dirblock->file_blocks[k]);
			if (snorlax == TBA) return NULL;
			if (file->fcb.is_dir != FIL) break;
			if (strcmp(filename, file->fcb.name) == 0) {
				
				handle->sfs = d->sfs;
				handle->fcb = file;
				handle->directory = d->dcb;
				handle->current_block = &file->header;
				handle->pos_in_file = 0;
								
				return handle;
			}			
			++k;
			++j;
		}
		++i;
	}
			
	return NULL;
	
}


// closes a file handle (destroyes it)
// RETURNS 0 on success, -1 if fails
int SimpleFS_close(FileHandle* f) {

	if (f == NULL)	return TBA;
	
	free(f->fcb);
	free(f);
	
	return 0; 
}

// useful to decide how much data take from the buffer
int SimpleFS_write_aux (int size, int remaining_space) {
	if (size <= remaining_space) return size;
	else return remaining_space;
}

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size) {
	
	// Setting and checking for DiskDriver and for the handle
	DiskDriver* disk = f->sfs->disk;
	if (disk == NULL) return TBA;
	
	if (f == NULL) return TBA;
	
	// Creating useful things
	int fbsize = BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader);
	int bsize = BLOCK_SIZE - sizeof(BlockHeader);
	
	int wdata = 0;
	
	int len = fbsize + (bsize * f->fcb->fcb.size_in_blocks);
	if (len < size + f->pos_in_file) len = size + f->pos_in_file;
	
	char array[len];
	
	// creating an handle clone that starts from the beginning of the file
	FileHandle* aux = (FileHandle*) malloc(sizeof(FileHandle));
	aux->sfs = f->sfs;
	aux->fcb = f->fcb;
	aux->directory = f->directory;
	aux->current_block = &f->fcb->header;
	aux->pos_in_file = 0;
	
	// Getting the first block
	FirstFileBlock* firstblock = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	FileBlock* fileblock = (FileBlock*) malloc(sizeof(FileBlock));
	int snorlax = DiskDriver_readBlock(disk, firstblock, aux->current_block->block_in_disk);
	if (snorlax == TBA) return ERROR_FS_FAULT;
	
	int taken_data = SimpleFS_write_aux(fbsize, size-wdata);
	strncpy(array+wdata, f->fcb->data, taken_data);
	wdata += taken_data;
	aux->pos_in_file += taken_data;
	
	int i = 1;
	if (aux->current_block->next_block != TBA) {
		snorlax = DiskDriver_readBlock(disk, fileblock, aux->current_block->next_block);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		while (i < aux->fcb->fcb.size_in_blocks) {
			taken_data = SimpleFS_write_aux(bsize, size-wdata);
			strncpy(array+wdata, f->fcb->data, taken_data);
			wdata += taken_data;
			aux->pos_in_file += taken_data;
			++i;			
			
			aux->current_block = &fileblock->header;
			if (aux->current_block->next_block == TBA) break;
			snorlax = DiskDriver_readBlock(disk, fileblock, aux->current_block->next_block);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			
		}
	}

	strcpy(array+f->pos_in_file, data);
	
//	printf ("fbsize: %d, bsize: %d, len: %d,(len-fbsize)/bsize: %d\n", fbsize, bsize, size, (size-fbsize)/bsize);

//	printf ("\nLast Visited Block\nBlock in disk: %d\nBlock in file: %d\n Previous Block: %d\nNext Block: %d\n\n",
//			aux->current_block->block_in_disk, aux->current_block->block_in_file, aux->current_block->previous_block, aux->current_block->next_block);

	int necessary_blocks = ((size + f->pos_in_file - fbsize) / bsize ) + 1;
		
	// Allocating all the blocks we need for the write of the entire file
	while (necessary_blocks - f->fcb->fcb.size_in_blocks >= 0 ) {

		// Getting the first free block on the disk
		int voyager = DiskDriver_getFreeBlock(disk, 0);
		if (voyager == TBA) return ERROR_FS_FAULT;
		FileBlock* block = (FileBlock*) malloc(sizeof(FileBlock));
		block->header.previous_block = aux->current_block->block_in_disk;
		block->header.next_block = TBA;
		block->header.block_in_file = aux->current_block->block_in_file +1;
		block->header.block_in_disk = voyager;
		memset(block->data, 0, bsize);
		
		// Writing the new block on the disk
		snorlax = DiskDriver_writeBlock(disk, block, block->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// Getting the current block and updating it
		// Current block is the first file block
		if (aux->current_block->block_in_disk == f->fcb->header.block_in_disk) {
			
			// Updating the header
			f->fcb->header.next_block = block->header.block_in_disk;
			
			// Updating the fcb
			f->fcb->fcb.size_in_blocks += 1;
			f->fcb->fcb.size_in_bytes += BLOCK_SIZE;
			
			// Writing on the disk
			snorlax = DiskDriver_writeBlock(disk, f->fcb, f->fcb->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;

		}
		// Current Block is a common block
		else {
			snorlax = DiskDriver_readBlock(disk, fileblock, aux->current_block->block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			
			// Updating the header
			fileblock->header.next_block = block->header.block_in_disk;
			
			// Writing on the disk
			snorlax = DiskDriver_writeBlock(disk, fileblock, aux->current_block->block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			
			// Updating the fcb
			snorlax = DiskDriver_readBlock(disk, firstblock, f->fcb->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			f->fcb->fcb.size_in_blocks += 1;
			f->fcb->fcb.size_in_bytes += BLOCK_SIZE;
			
			// Writing on the disk
			snorlax = DiskDriver_writeBlock(disk, f->fcb, f->fcb->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
		}
		
		// Updating current block
		aux->current_block = &block->header;
	}

//	printf ("\nLast Visited Block\nBlock in disk: %d\nBlock in file: %d\n Previous Block: %d\nNext Block: %d\n\n",
//			aux->current_block->block_in_disk, aux->current_block->block_in_file, aux->current_block->previous_block, aux->current_block->next_block);

//	printf ("\nFirstBlock\nBlock in disk: %d\nBlock in file: %d\n Previous Block: %d\nNext Block: %d\n\n", 
//			f->fcb->header.block_in_disk, f->fcb->header.block_in_file, f->fcb->header.previous_block, f->fcb->header.next_block);
	
	// Time to write on the file!
	aux->current_block = &f->fcb->header;
	aux->pos_in_file = 0;
	wdata = 0;
	
	while (aux->pos_in_file < size+f->pos_in_file) {
				
		// if we are in the first file block
		if (aux->current_block->block_in_disk == f->fcb->header.block_in_disk) {
			
			taken_data = SimpleFS_write_aux(fbsize, size+f->pos_in_file-wdata);
			strncpy(f->fcb->data, array+aux->pos_in_file, taken_data);
			wdata += taken_data;
			aux->pos_in_file += taken_data;
			
			// writing on the disk
			snorlax = DiskDriver_writeBlock(disk, f->fcb, f->fcb->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
		}
		// if we are in a common block
		else {
			
			snorlax = DiskDriver_readBlock(disk, fileblock, aux->current_block->block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			taken_data = SimpleFS_write_aux(bsize, size+f->pos_in_file-wdata);
			strncpy(fileblock->data, array+aux->pos_in_file, taken_data);
			wdata += taken_data;
			
			aux->pos_in_file += taken_data;
			
			// writing on the disk
			snorlax = DiskDriver_writeBlock(disk, fileblock, aux->current_block->block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			
		}
		
		// updating the current block
		if (aux->current_block->next_block != TBA) {
			FileBlock* block = (FileBlock*) malloc(sizeof(FileBlock));
			snorlax = DiskDriver_readBlock(disk, block, aux->current_block->next_block);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			aux->current_block = &block->header;
		}
	}
	
	wdata = size;
	
	f->current_block = aux->current_block;
	f->pos_in_file = aux->pos_in_file;
	
	return wdata;
}

// prints all blocks of a file
void SimpleFS_printFileBlocks (FileHandle* f) {
	int list[f->fcb->fcb.size_in_blocks];
	FileBlock* fileblock = (FileBlock*) malloc(sizeof(FileBlock));
	int i = 0;
	list[i] = f->fcb->header.block_in_disk;
	if (f->fcb->header.next_block != TBA) {
		int snorlax = DiskDriver_readBlock(f->sfs->disk, fileblock, f->fcb->header.next_block);
		if (snorlax == TBA) return;
		++i;
		while (i < f->fcb->fcb.size_in_blocks) {
			list[i] = fileblock->header.block_in_disk;
			if (fileblock->header.next_block != TBA) {
				snorlax = DiskDriver_readBlock(f->sfs->disk, fileblock, fileblock->header.next_block);
				if (snorlax == TBA) return;
			}
			++i;
		}
	}
	printf ("\nFile %s is composed by blocks: ", f->fcb->fcb.name);
	printf ("{ ");
	for (int j = 0; j < i; ++j) {
		printf ("%d ", list[j]);
	}
	printf ("}\n");
	free(fileblock);
}

// reads in the file, at current position size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size) {
	
	// Setting and checking for DiskDriver and for the handle
	DiskDriver* disk = f->sfs->disk;
	if (disk == NULL) return TBA;
	
	if (f == NULL) return TBA;
	
	// Creating useful things
	int fbsize = BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader);
	int bsize = BLOCK_SIZE - sizeof(BlockHeader);
		
	char buffer[fbsize + (f->fcb->fcb.size_in_blocks * bsize)];// = (char*) malloc(sizeof(char) * (fbsize + (f->fcb->fcb.size_in_blocks * bsize)));
	int rdata = 0;
	int potato = 0;
		
	memcpy(buffer+potato, f->fcb->data, fbsize);
	rdata += fbsize;
	potato += fbsize;

	FileBlock* fileblock = (FileBlock*) malloc(sizeof(FileBlock));
	if (f->fcb->header.next_block == TBA) return rdata;
	int snorlax = DiskDriver_readBlock(disk, fileblock, f->fcb->header.next_block);
	if (snorlax == TBA) return rdata;

	
	int i = 1;
	while (i < f->fcb->fcb.size_in_blocks) {
		potato = SimpleFS_write_aux(size-rdata, bsize);
		memcpy(buffer+rdata, fileblock->data, potato);
		rdata += potato;
		if (fileblock->header.next_block != TBA) {
			snorlax = DiskDriver_readBlock(disk, fileblock, fileblock->header.next_block);
			if (snorlax == TBA) return rdata;
		}
		++i;
	}

	memcpy(data, buffer, size);
	free (fileblock);
	
	return rdata;
}

// returns the number of bytes read (moving the current pointer to pos)
// returns pos on success
// -1 on error (file too short)
int SimpleFS_seek(FileHandle* f, int pos) {
	
	if (f == NULL) return TBA;
	
	// Creating useful things
	int fbsize = BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader);
	int bsize = BLOCK_SIZE - sizeof(BlockHeader);
	int count = 0;
	
	// seeking
	int i = 0;
	while (count < fbsize) {
		if (f->fcb->data[i] != EOF) {
			if ( i == pos) {
				f->pos_in_file = count;
				f->current_block = &f->fcb->header;
				return count;
			}
			++count;
			++i;
		}
		else return TBA;
	}
	
	if (f->fcb->header.next_block == TBA) return count;
	FileBlock* fileblock = (FileBlock*) malloc(sizeof(FileBlock));
	int snorlax = DiskDriver_readBlock(f->sfs->disk, fileblock, f->fcb->header.next_block); 
	if (snorlax == TBA) return count;
	
	int j = 1;	
	while (j < f->fcb->fcb.size_in_blocks) {
		i = 0;
		while (i < bsize) {
			if (fileblock->data[i] != EOF) {
				if (i + fbsize + (bsize * j-1) == pos) {
					f->pos_in_file = count;
					f->current_block = &fileblock->header;
					return count;
				}
				++count;
				++i;
			}
			else return TBA;
		}
		++j;
	}
	
	if (pos >= count) return TBA;
	else return count;	
	
}

// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
 int SimpleFS_changeDir(DirectoryHandle* d, char* dirname) {
	 
	if (d == NULL) return ERROR_FS_FAULT;
	
	// Setting and checking for DiskDriver
	DiskDriver* disk = d->sfs->disk;
	if (disk == NULL) return ERROR_FS_FAULT;
	
	if (strcmp(dirname, "..") == 0) {
		
		FirstDirectoryBlock* parent = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
		int snorlax = DiskDriver_readBlock(disk, parent, d->dcb->fcb.directory_block);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// well done! Just go back
		d->dcb = parent;
		d->current_block = &parent->header;
		d->pos_in_dir = 0;
		d->pos_in_block = 0;
			
		return 0;
	}
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	// Creating a copy of dirhandle
	DirectoryHandle* dirhandle = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
	dirhandle->sfs = d->sfs;
	dirhandle->dcb = d->dcb;
	dirhandle->directory = d->directory;
	dirhandle->current_block = &d->dcb->header;
	dirhandle->pos_in_dir = 0;
	dirhandle->pos_in_block = 0;
	
	// Creating a firstdirblock in where to store the read data
	FirstDirectoryBlock* iterator = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
	
	// Scanning in the first block if there's a directory with the same name
	while (dirhandle->pos_in_block < fbsize) {
		
		if (d->dcb->file_blocks[dirhandle->pos_in_block] == TBA || d->dcb->file_blocks[dirhandle->pos_in_block] > disk->header->num_blocks ) break;
		
		int snorlax = DiskDriver_readBlock(disk, iterator, d->dcb->file_blocks[dirhandle->pos_in_block]);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// if the block is DIR and it's a firstdirblock
		if (iterator->fcb.is_dir == DIR) {
			
			if (strcmp(dirname, iterator->fcb.name) == 0) {
				
				// well done! Just update the original dirhandle
				d->directory = d->dcb;
				d->dcb = iterator;
				d->current_block = &iterator->header;
				d->pos_in_dir = 0;
				d->pos_in_block = 0;
				
				return 0;
			}
	
		}
		
		++dirhandle->pos_in_block;
	}
		
	// Scanning other blocks (if are there)
	int snorlax = TBA;
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	if (dirhandle->current_block->next_block != TBA) {
		snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->next_block);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}	

	// Scanning this new block
	while (dirhandle->pos_in_dir < dirhandle->dcb->fcb.size_in_blocks) {
		
		// Updating current block and current position
		dirhandle->pos_in_block = 0;
		dirhandle->current_block = &dirblock->header;
		dirhandle->pos_in_dir = dirblock->header.block_in_file;
		
		while (dirhandle->pos_in_block < bsize) {
			
			if (dirblock->file_blocks[dirhandle->pos_in_block] == TBA || dirblock->file_blocks[dirhandle->pos_in_block] > disk->header->num_blocks ) break;
									
			snorlax = DiskDriver_readBlock(disk, iterator, dirblock->file_blocks[dirhandle->pos_in_block]);
			if (snorlax == TBA) return TBA;
			
			// if the block is DIR and it's a firstdirblock
			if (iterator->fcb.is_dir == DIR) {
				
				if (strcmp(dirname, iterator->fcb.name) == 0) {
				
				// well done! Just update the original dirhandle
				d->directory = d->dcb;
				d->dcb = iterator;
				d->current_block = &iterator->header;
				d->pos_in_dir = 0;
				d->pos_in_block = 0;
				
				return 0;
				}
					
			}
			++dirhandle->pos_in_block;
		}
		
		if (dirhandle->current_block->next_block == TBA) {
			break;
		}
		else {
			snorlax = DiskDriver_readBlock(disk, dirblock, dirblock->header.next_block);
			if (snorlax == TBA) return TBA;
		}
		
	}
	
	free(iterator);
	free(dirblock);
	free(dirhandle);
	
	return ERROR_FS_FAULT; 
 }

// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname) {
	
	if (d == NULL) return TBA;
	
	// Setting and checking for DiskDriver
	DiskDriver* disk = d->sfs->disk;
	if (disk == NULL) return TBA;
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	// Creating a copy of dirhandle
	DirectoryHandle* dirhandle = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
	dirhandle->sfs = d->sfs;
	dirhandle->dcb = d->dcb;
	dirhandle->directory = d->directory;
	dirhandle->current_block = &d->dcb->header;
	dirhandle->pos_in_dir = 0;
	dirhandle->pos_in_block = 0;
	
	// Creating a firstdirblock in where to store the read data
	FirstDirectoryBlock* iterator = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
	
	// Scanning in the first block if there's a directory with the same name
	while (dirhandle->pos_in_block < fbsize) {
		
		if (d->dcb->file_blocks[dirhandle->pos_in_block] == TBA || d->dcb->file_blocks[dirhandle->pos_in_block] > disk->header->num_blocks ) break;
		
		int snorlax = DiskDriver_readBlock(disk, iterator, d->dcb->file_blocks[dirhandle->pos_in_block]);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// if the block is DIR and it's a firstdirblock
		if (iterator->fcb.is_dir == DIR) {
			
			if (strcmp(dirname, iterator->fcb.name) == 0) {
				printf ("ALREADY EXISTS A DIR WITH THE SAME NAME!\n");
				return TBA;
			}
			
		}
		
		++dirhandle->pos_in_block;
	}
	
	// Scanning other blocks (if are there)
	int snorlax = TBA;
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	if (dirhandle->current_block->next_block != TBA) {
		snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->next_block);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}	
	
	// Scanning this new block
	while (dirhandle->pos_in_dir < dirhandle->dcb->fcb.size_in_blocks) {
		
		// Updating current block and current position
		dirhandle->pos_in_block = 0;
		dirhandle->current_block = &dirblock->header;
		dirhandle->pos_in_dir = dirblock->header.block_in_file;
		
		while (dirhandle->pos_in_block < bsize) {
			
			if (dirblock->file_blocks[dirhandle->pos_in_block] <= TBA || dirblock->file_blocks[dirhandle->pos_in_block] > disk->header->num_blocks ) break;
									
			snorlax = DiskDriver_readBlock(disk, iterator, dirblock->file_blocks[dirhandle->pos_in_block]);
			if (snorlax == TBA) return TBA;
			
			// if the block is DIR and it's a firstdirblock
			if (iterator->fcb.is_dir == DIR) {
				
				if (strcmp(dirname, iterator->fcb.name) == 0) {
					printf ("ALREADY EXISTS A DIR WITH THE SAME NAME!\n");
					return TBA;
				}
				
			}
			++dirhandle->pos_in_block;
		}
		
		if (dirhandle->current_block->next_block == TBA) {
			break;
		}
		else {
			snorlax = DiskDriver_readBlock(disk, dirblock, dirblock->header.next_block);
			if (snorlax == TBA) return TBA;
		}
		
	}

/*	
	printf ("First block: %d\nParent dir's block: %d\nCurrent Block: %d\nPos in dir: %d\nPos in block: %d\n\n", dirhandle->dcb->header.block_in_disk, dirhandle->directory->header.block_in_disk, dirhandle->current_block->block_in_disk, dirhandle->pos_in_dir, dirhandle->pos_in_block);
*/
	
	// At this point i know that there are no directories with the same name
	// If the function arrives at this point, means that there are no duplicates
	
	// if voyager == TBA means that there's need to create a new block
	// don't care to fill the removed files places
	int voyager = DiskDriver_getFreeBlock(disk, 0);
	if (voyager == TBA) {
		//memset(dirblock, 0, BLOCK_SIZE);
		
		// setting the dirblock
		voyager = DiskDriver_getFreeBlock(disk, 0);
		if (voyager == TBA) return ERROR_FS_FAULT;
		dirblock->header.block_in_disk = voyager;
		dirblock->header.previous_block = dirhandle->current_block->block_in_disk;
		dirblock->header.next_block = TBA;
		dirblock->header.block_in_file = dirhandle->pos_in_dir + 1;
		memset(dirblock->file_blocks, TBA, bsize);
		
		// writing the block on the disk
		snorlax = DiskDriver_writeBlock(disk, dirblock, voyager);
		
		// updating the original dirhandle
		d->dcb->fcb.size_in_blocks += 1;
		d->dcb->fcb.size_in_bytes += BLOCK_SIZE;
		d->dcb->num_entries += 1;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// updating the actual dirhandle
		dirhandle->current_block = &dirblock->header;
		dirhandle->pos_in_dir += 1;
		dirhandle->pos_in_block = 0;
	}
	
	// Time to create the new directory
	memset(iterator, 0, BLOCK_SIZE);
	
	iterator->header.previous_block = TBA;
	iterator->header.next_block = TBA;
	iterator->header.block_in_file = 0;
	iterator->header.block_in_disk = voyager;
	
	iterator->fcb.directory_block = d->dcb->header.block_in_disk;
	iterator->fcb.block_in_disk = voyager;
	strcpy(iterator->fcb.name, dirname);
	iterator->fcb.size_in_bytes = BLOCK_SIZE;
	iterator->fcb.size_in_blocks = 1;
	iterator->fcb.is_dir = DIR;
	
	iterator->num_entries = 0;
	
	memset(iterator->file_blocks, TBA, fbsize);
	
	// writing on the disk
	snorlax = DiskDriver_writeBlock(disk, iterator, iterator->header.block_in_disk);
	if (snorlax == TBA) return ERROR_FS_FAULT;
	
	// Inserting this new directory in the actual directory
	if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
		// we are in the first dir block
		d->dcb->file_blocks[dirhandle->pos_in_block] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}
	else {
		// we are in a common dirblock
		snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		dirblock->file_blocks[dirhandle->pos_in_block] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, dirblock, dirblock->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}
	
	// updating the directory handle
	d->dcb->num_entries += 1;
	snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
	if (snorlax == TBA) return ERROR_FS_FAULT;
	
	free(iterator);
	free(dirblock);
	free(dirhandle);	
	
	return 0;	
}

// saves all blocks of a file
void SimpleFS_remove_aux_file (FileHandle* f, int* list) {
	
	FileBlock* fileblock = (FileBlock*) malloc(sizeof(FileBlock));
	int i = 0;
	list[i] = f->fcb->header.block_in_disk;
	if (f->fcb->header.next_block != TBA) {
		int snorlax = DiskDriver_readBlock(f->sfs->disk, fileblock, f->fcb->header.next_block);
		if (snorlax == TBA) return;
		++i;
		while (i < f->fcb->fcb.size_in_blocks) {
			list[i] = fileblock->header.block_in_disk;
			if (fileblock->header.next_block != TBA) {
				snorlax = DiskDriver_readBlock(f->sfs->disk, fileblock, fileblock->header.next_block);
				if (snorlax == TBA) return;
			}
			++i;
		}
	}
	free(fileblock);
	
	return;
}

// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(SimpleFS* fs, char* filename) {
	
	// Checking for DirectoryHandle existance
	if (fs == NULL) {
		return ERROR_FS_FAULT;
	}
	
	// Setting and checking for DiskDriver
	DiskDriver* disk = fs->disk;
	if (disk == NULL) return ERROR_FS_FAULT;
	
	// Creating a void block
	FirstFileBlock* block = (void*) malloc(BLOCK_SIZE);
	int snorlax = TBA;
	
	// Scanning all the blocks in the disk
	for (int i = 0; i < disk->header->num_blocks; ++i) {
		
		snorlax = DiskDriver_readBlock(disk, block, i);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// if true, means that is a first file block or a first dir block
		if (block->header.previous_block == TBA) {
			// if we found our file
			if (strcmp(block->fcb.name, filename) == 0) {
				// Checking if it's a FIL..
				if (block->fcb.is_dir == FIL) {
					// creating a rudimental filehandle
					FileHandle* f = (FileHandle*) malloc(sizeof(FileHandle));
					f->sfs = fs;
					f->fcb = block;
					f->directory = NULL;
					f->current_block = &block->header;
					f->pos_in_file = 0;
					
					int list[block->fcb.size_in_blocks];
					SimpleFS_remove_aux_file(f, list);
					
					// now list contains all the block numbers of the file
					for (int j = 0; j < block->fcb.size_in_blocks; ++j) {						
						snorlax = DiskDriver_freeBlock(disk, list[j]);
						if (snorlax == TBA) return ERROR_FS_FAULT;
					}
					free(f);
				}
				// or a DIR
				else {
					
				}				
			}			
		}
				
	}
	
	return 0;
}
