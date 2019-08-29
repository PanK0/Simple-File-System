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

// Gets the first free position in a directory block array 
// to avoid to fill a directory with deleted files
// returns the number of the first free position,
// returns -1 if there are no free blocks
int SimpleFS_get13pos (DirectoryHandle* d, BlockHeader* header) {
	
	if (d == NULL) return TBA;
	
	void* aux_block = (void*) malloc(BLOCK_SIZE);
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	if (header->block_in_disk == d->dcb->header.block_in_disk) {
		FirstDirectoryBlock* fdb = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
		int snorlax = DiskDriver_readBlock(d->sfs->disk, fdb, header->block_in_disk);
		if (snorlax == TBA) return TBA;
		for (int i = 0; i < fbsize; ++i) {
			if (fdb->file_blocks[i] == TBA) {
				fdb = NULL;
				free(fdb);
				return i;
			}
			if (fdb->file_blocks[i] < d->sfs->disk->header->num_blocks) {
				snorlax = DiskDriver_readBlock(d->sfs->disk, aux_block, fdb->file_blocks[i]);			
				if (snorlax == TBA) {
					fdb = NULL;
					free(fdb);
					return i;
				}
			}
		}
	}
	else {
		DirectoryBlock* db = (DirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
		int snorlax = DiskDriver_readBlock(d->sfs->disk, db, header->block_in_disk);
		if (snorlax == TBA) return TBA;
		for (int i = 0; i < bsize; ++i) {
			if (db->file_blocks[i] == TBA) {
				db = NULL;
				free(db);
				return i;
			}
			if (db->file_blocks[i] < d->sfs->disk->header->num_blocks) {
				snorlax = DiskDriver_readBlock(d->sfs->disk, aux_block, db->file_blocks[i]);
				if (snorlax == TBA) {
					db = NULL;
					free(db);
					return i;
				}
			}
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
	
	// FirstDirectoryBlock -> size of file_blocks
	int fbsize = (BLOCK_SIZE
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int);
	// DirectoryBlock -> size of file_blocks
	int bsize = (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int);
	
	// Checking for remaining free blocks
	// Might need 2 free block to store the file block and an additional directory block
	if (disk->header->free_blocks <= 1) return NULL;
	
	// Creating a copy of dirhandle
	DirectoryHandle* dirhandle = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
	dirhandle->sfs = d->sfs;
	dirhandle->dcb = d->dcb;
	dirhandle->directory = d->directory;
	dirhandle->current_block = &d->dcb->header;
	dirhandle->pos_in_dir = 0;
	dirhandle->pos_in_block = 0;
	
	// Creating a first file block in where to store the read data
	FirstFileBlock* iterator = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));

	DirectoryBlock* audir = (DirectoryBlock*) malloc(sizeof(FirstFileBlock));
	int count = 0;

	// Scanning in the first block if there's a file with the same name
	while (dirhandle->pos_in_dir == 0 && dirhandle->pos_in_block < fbsize) {

		if (d->dcb->file_blocks[dirhandle->pos_in_block] != TBA && d->dcb->file_blocks[dirhandle->pos_in_block] < disk->header->num_blocks ) {
			
			int snorlax = DiskDriver_readBlock(disk, iterator, d->dcb->file_blocks[dirhandle->pos_in_block]);
			if (snorlax == TBA) return NULL;
			
			// if the block is FIL and it's a firstdirblock
			if (snorlax == 0) {
				if ( iterator->fcb.is_dir == FIL) {
				++count;
					if (strcmp(filename, iterator->fcb.name) == 0) {
						printf ("ALREADY EXISTS A FILE WITH THE SAME NAME! 1 %s\n", iterator->fcb.name);
						iterator = NULL;
						free (iterator);
						dirhandle = NULL;
						free (dirhandle);
						audir = NULL;
						free (audir);
						return NULL;
					}
				}
				
			}
		}		
		++dirhandle->pos_in_block;
	}
	
	// Scanning other blocks (if are there)
	int snorlax = TBA;
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	if (dirhandle->current_block->next_block != TBA) {
		// Scanning from this new block
		while (dirhandle->current_block->next_block != TBA) {
			
			count = 0;
			dirhandle->pos_in_block = 0;		
			snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->next_block);
			if (snorlax == TBA) return NULL;
			dirhandle->current_block = &dirblock->header;
			dirhandle->pos_in_dir += 1;
			
			while (dirhandle->pos_in_block < bsize) {
				
				if (dirblock->file_blocks[dirhandle->pos_in_block] != TBA && dirblock->file_blocks[dirhandle->pos_in_block] < disk->header->num_blocks) {
		
					snorlax = DiskDriver_readBlock(disk, iterator, dirblock->file_blocks[dirhandle->pos_in_block]);
					if (snorlax == TBA) return NULL;
					
					// if the block is FIL and it's a firstdirblock
					if (snorlax == 0) {
						++count;
						if (iterator->fcb.is_dir == FIL) {
						
							if (strcmp(filename, iterator->fcb.name) == 0) {
								printf ("ALREADY EXISTS A FILE WITH THE SAME NAME! 2 %s\n", iterator->fcb.name);
								iterator = NULL;
								free (iterator);
								dirhandle = NULL;
								free (dirhandle);
								dirblock = NULL;
								free (dirblock);
								audir = NULL;
								free (audir);
								return NULL;
							}
						}						
					}
				}
				dirhandle->pos_in_block += 1;
			}
		}	
	}

	// At this point i know that there are no files with the same name
	// If the function arrives at this point, means that there are no duplicates
	
	//must determinate if there's need to create a new dirblock
	int flag = TBA;
	if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
		if (count >= fbsize) flag = 1;
	}
	else {
		if (count >= bsize) {
			flag = 1;
		}
	}
	int voyager = DiskDriver_getFreeBlock(disk, 0);
	if (flag != TBA) {
		memset(audir, 0, BLOCK_SIZE);
		audir->header.block_in_disk = voyager;
		audir->header.previous_block = dirhandle->current_block->block_in_disk;
		audir->header.next_block = TBA;
		audir->header.block_in_file = dirhandle->pos_in_dir +1;
//		memset(dirblock->file_blocks, TBA, bsize);
		for (int k = 0; k < bsize; ++k) {
			audir->file_blocks[k] = TBA;
		}
		
		// writing the block on the disk
		snorlax = DiskDriver_writeBlock(disk, audir, voyager);
		
		// checking and updating the current block
		if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
			d->dcb->header.next_block = audir->header.block_in_disk;
			snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
			if (snorlax == TBA) return NULL;
		}
		else {
			DirectoryBlock* gre = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
			snorlax = DiskDriver_readBlock(disk, gre, dirhandle->current_block->block_in_disk);
			if (snorlax == TBA) return NULL;
			gre->header.next_block = audir->header.block_in_disk;
			snorlax = DiskDriver_writeBlock(disk, gre, gre->header.block_in_disk);
			if (snorlax == TBA) return NULL;
			//free(gre);
		}
		
		// updating the original dirhandle
		d->dcb->fcb.size_in_blocks += 1;
		d->dcb->fcb.size_in_bytes += BLOCK_SIZE;
//		d->current_block = &dirblock->header;
//		d->pos_in_dir += 1;
//		d->pos_in_block = 0;
//		d->dcb->num_entries += 1;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return NULL;
		
		// updating the actual dirhandle
		dirhandle->current_block = &dirblock->header;
		dirhandle->pos_in_dir += 1;
		dirhandle->pos_in_block = 0;
		
	}

	// Time to create the new file
	voyager = DiskDriver_getFreeBlock(disk, 0);
	
	memset(iterator, 0, BLOCK_SIZE);
	
	iterator->header.previous_block = TBA;
	iterator->header.next_block = TBA;
	iterator->header.block_in_file = 0;
	iterator->header.block_in_disk = voyager;
	
	iterator->fcb.directory_block = d->dcb->header.block_in_disk;
	iterator->fcb.block_in_disk = voyager;
	strcpy(iterator->fcb.name, filename);
	iterator->fcb.size_in_bytes = BLOCK_SIZE;
	iterator->fcb.size_in_blocks = 1;
	iterator->fcb.is_dir = FIL;
	
	memset(iterator->data, TBA, fbsize);
	
	// writing on the disk
	snorlax = DiskDriver_writeBlock(disk, iterator, iterator->header.block_in_disk);
	if (snorlax == TBA) return NULL;
	
	voyager = SimpleFS_get13pos(d, dirhandle->current_block);
	
	// inserting this new file in the actual directory
	if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
		// we are in the first dir block
		d->dcb->file_blocks[voyager] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return NULL;
	}
	else {
		// we are in a common block
		snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->block_in_disk);
		if (snorlax == TBA) return NULL;
		dirblock->file_blocks[voyager] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, dirblock, dirblock->header.block_in_disk);
		if (snorlax == TBA) return NULL;
	}
	
	// updating the directory handle
	d->dcb->num_entries += 1;
	snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
	if (snorlax == TBA) return NULL;
	
	// filling the filehandle
	FileHandle* handle = (FileHandle*) malloc(sizeof(FileHandle));
	handle->sfs = d->sfs;
	handle->fcb = iterator;
	handle->directory = d->dcb;
	handle->current_block = &iterator->header;
	handle->pos_in_file = 0;

	dirblock = NULL;
	free (dirblock);
	dirhandle = NULL;
	free (dirhandle);
	audir = NULL;
 	free (audir);
	
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
	
	iterator = NULL;
	free(iterator);
	dirblock = NULL;
	free(dirblock);

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
	
	iterator = d->current_block;
	int i = 0;
	int count = 0;
	
	while (i < d->dcb->fcb.size_in_blocks) {
		blocklist_array[i] = iterator->block_in_disk;
		if (iterator->next_block != TBA) {
			int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, iterator->next_block);
			if (snorlax == TBA) return snorlax;
		}		
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
	FirstFileBlock* f = (FirstFileBlock*) malloc(sizeof(FirstFileBlock));
	while (j < fbsize) {
		if (d->dcb->file_blocks[j] != TBA && d->dcb->file_blocks[j] < d->sfs->disk->header->num_blocks) {
			int snorlax = DiskDriver_readBlock(d->sfs->disk, f, d->dcb->file_blocks[j]);
			if (snorlax != TBA) {
				strcpy(names[j], f->fcb.name);
				++count;
			}
		}
		++j;
	}
	
	i = 1;
	//printf ("BBBBBBB i : %d, blocklist_len : %d\n", i, blocklist_len);
	while (i < blocklist_len) {
		int k = 0;
		int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, blocklist_array[i]);
		if (snorlax == TBA) return snorlax;
		while (k < bsize) {
			if (dirblock->file_blocks[k] != TBA) {
				snorlax = DiskDriver_readBlock(d->sfs->disk, f, dirblock->file_blocks[k]);
				if (snorlax != TBA) {
					strcpy(names[j], f->fcb.name);
					++count;
				}
			}
			++k;
			++j;
		}
		++i;
	}
	
	iterator = NULL;
	free (iterator);
	dirblock = NULL;
	free (dirblock);
	f = NULL;
	free (f);
	
	return count;
	
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
			
			handle = NULL;
			free (handle);
			dirblock = NULL;
			free (dirblock);
			iterator = NULL;
			free (iterator);
			file = NULL;
			free (file);
			
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
				
				handle = NULL;
				free (handle);
				dirblock = NULL;
				free (dirblock);
				iterator = NULL;
				free (iterator);
				file = NULL;
				free (file);
								
				return handle;
			}			
			++k;
			++j;
		}
		++i;
	}
	
	handle = NULL;
	free (handle);
	dirblock = NULL;
	free (dirblock);
	iterator = NULL;
	free (iterator);
	file = NULL;
	free (file);
			
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
	if (f == NULL) return TBA;
	
	DiskDriver* disk = f->sfs->disk;
	if (disk == NULL) return TBA;
	
	
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
	
	aux = NULL;
	free (aux);
	firstblock = NULL;
	free (firstblock);
	fileblock = NULL;
	free (fileblock);
	
	return wdata;
}

// prints all blocks of a file
void SimpleFS_printFileBlocks (FileHandle* f) {
	
	if (f == NULL) {
		printf ("Given Handler is NULL - from SimpleFS_printFileBlocks()\n");
		return;
	}
	
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
	
	fileblock = NULL;
	free(fileblock);
}

// reads in the file, at current position size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size) {
	
	// Setting and checking for DiskDriver and for the handle
	if (f == NULL) return TBA;
	
	DiskDriver* disk = f->sfs->disk;
	if (disk == NULL) return TBA;
	
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
	
	fileblock = NULL;
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
	
	fileblock = NULL;
	free (fileblock);
	
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
				
				dirhandle = NULL;
				free (dirhandle);
				iterator = NULL;
				free (iterator);
				
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
				
				dirblock = NULL;
				free (dirblock);
				dirhandle = NULL;
				free (dirhandle);
				iterator = NULL;
				free (iterator);
				
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
	
	dirblock = NULL;
	free (dirblock);
	dirhandle = NULL;
	free (dirhandle);
	iterator = NULL;
	free (iterator);
	
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
	
	// Checking for remaining free blocks
	// Might need 2 free block to store the file block and an additional directory block
	if (disk->header->free_blocks <= 1) return ERROR_FS_FAULT;
	
	
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
	
	DirectoryBlock* audir = (DirectoryBlock*) malloc(sizeof(FirstFileBlock));
	int count = 0;
	
	// Scanning in the first block if there's a directory with the same name
	while (dirhandle->pos_in_block < fbsize) {
		
		if (d->dcb->file_blocks[dirhandle->pos_in_block] != TBA  ) {
		
			int snorlax = DiskDriver_readBlock(disk, iterator, d->dcb->file_blocks[dirhandle->pos_in_block]);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			
			// if the block is DIR and it's a firstdirblock
			if (snorlax == 0) {
				++count;
				if (iterator->fcb.is_dir == DIR) {
					if (strcmp(dirname, iterator->fcb.name) == 0) {
						printf ("ALREADY EXISTS A DIR WITH THE SAME NAME!\n");
						
						iterator = NULL;
						free (iterator);
						dirhandle = NULL;
						free (dirhandle);
						audir = NULL;
						free (audir);
						return TBA;
					}
				}
				
			}
		}		
		++dirhandle->pos_in_block;
	}
	
	// Scanning other blocks (if are there)
	int snorlax = TBA;
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
	if (dirhandle->current_block->next_block != TBA) {	
	
		// Scanning this new block
		while (dirhandle->current_block->next_block != TBA) {
			
			count = 0;
			dirhandle->pos_in_block = 0;
			snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->next_block);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			dirhandle->current_block = &dirblock->header;
			dirhandle->pos_in_dir += 1;
			
			while (dirhandle->pos_in_block < bsize) {
				if (dirblock->file_blocks[dirhandle->pos_in_block] != TBA) {
										
					snorlax = DiskDriver_readBlock(disk, iterator, dirblock->file_blocks[dirhandle->pos_in_block]);
					if (snorlax == TBA) return TBA;
					
					// if the block is DIR and it's a firstdirblock
					if (snorlax == 0) {
						++count;
						if ( iterator->fcb.is_dir == DIR) {
							if (strcmp(dirname, iterator->fcb.name) == 0) {
								printf ("ALREADY EXISTS A DIR WITH THE SAME NAME!\n");
								
								iterator = NULL;
								free (iterator);
								dirhandle = NULL;
								free (dirhandle);
								dirblock = NULL;
								free (dirblock);
								return TBA;
							}
						}				
					}
				}
				dirhandle->pos_in_block += 1;			
			}
		}
	}

/*	
	printf ("First block: %d\nParent dir's block: %d\nCurrent Block: %d\nPos in dir: %d\nPos in block: %d\n\n", dirhandle->dcb->header.block_in_disk, dirhandle->directory->header.block_in_disk, dirhandle->current_block->block_in_disk, dirhandle->pos_in_dir, dirhandle->pos_in_block);
*/
	
	// At this point i know that there are no directories with the same name
	// If the function arrives at this point, means that there are no duplicates
	
	// Must determinate if there's need to create a new dirblock
	int flag = TBA;
	if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
		if (count >= fbsize) flag = 1;
	}
	else {
		if (count >= bsize) flag = 1;
	}
	int voyager = DiskDriver_getFreeBlock(disk, 0);
	if (flag != TBA) {
		
		// setting the audir block
		memset(audir, 0, BLOCK_SIZE);
		audir->header.block_in_disk = voyager;
		audir->header.previous_block = dirhandle->current_block->block_in_disk;
		audir->header.next_block = TBA;
		audir->header.block_in_file = dirhandle->pos_in_dir + 1;
		memset(audir->file_blocks, TBA, bsize);
//		for (int k = 0; k < bsize; ++k) {
//			audir->file_blocks[k] = TBA;
//		}
		
		// writing the block on the disk
		snorlax = DiskDriver_writeBlock(disk, audir, voyager);
		
		// checking and updating the current block
		if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
			d->dcb->header.next_block = audir->header.block_in_disk;
			snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
		}
		else {
			DirectoryBlock* gre = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));
			snorlax = DiskDriver_readBlock(disk, gre, dirhandle->current_block->block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			gre->header.next_block = audir->header.block_in_disk;
			snorlax = DiskDriver_writeBlock(disk, gre, gre->header.block_in_disk);
			if (snorlax == TBA) return ERROR_FS_FAULT;
			gre = NULL;
			free(gre);
		}
		
		// updating the original dirhandle
		d->dcb->fcb.size_in_blocks += 1;
		d->dcb->fcb.size_in_bytes += BLOCK_SIZE;
//		d->current_block = &dirblock->header;
//		d->pos_in_dir += 1;
//		d->pos_in_block = 0;
//		d->dcb->num_entries += 1;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		
		// updating the actual dirhandle
		dirhandle->current_block = &dirblock->header;
		dirhandle->pos_in_dir += 1;
		dirhandle->pos_in_block = 0;
	}
	
	// Time to create the new directory
	voyager = DiskDriver_getFreeBlock(disk, 0);
	
	//memset(iterator, 0, BLOCK_SIZE);
	
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
	
//	memset(iterator->file_blocks, TBA, fbsize);
	for (int k = 0; k < fbsize; ++k) {
		iterator->file_blocks[k] = TBA;
	}
	
	// writing on the disk
	snorlax = DiskDriver_writeBlock(disk, iterator, iterator->header.block_in_disk);
	if (snorlax == TBA) return ERROR_FS_FAULT;
	
	voyager = SimpleFS_get13pos(d, dirhandle->current_block);
	
	// Inserting this new directory in the actual directory
	if (dirhandle->current_block->block_in_disk == d->dcb->header.block_in_disk) {
		// we are in the first dir block
		d->dcb->file_blocks[voyager] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}
	else {
		// we are in a common dirblock
		snorlax = DiskDriver_readBlock(disk, dirblock, dirhandle->current_block->block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
		dirblock->file_blocks[voyager] = iterator->header.block_in_disk;
		snorlax = DiskDriver_writeBlock(disk, dirblock, dirblock->header.block_in_disk);
		if (snorlax == TBA) return ERROR_FS_FAULT;
	}
	
	// updating the directory handle
	d->dcb->num_entries += 1;
	snorlax = DiskDriver_writeBlock(disk, d->dcb, d->dcb->header.block_in_disk);
	if (snorlax == TBA) return ERROR_FS_FAULT;
	
	iterator = NULL;
	free(iterator);
	dirblock = NULL;
	free(dirblock);
	dirhandle = NULL;
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
	
	fileblock = NULL;
	free (fileblock);
	
	return;
}

// saves all blocks of a directory
void SimpleFS_remove_aux_dir (DirectoryHandle* d, int* list) {
	
	DirectoryBlock* dirblock = (DirectoryBlock*) malloc (sizeof(DirectoryBlock));
	int i = 0;
	list[i] = d->dcb->header.block_in_disk;
	if (d->dcb->header.next_block != TBA) {
		int snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, d->dcb->header.next_block);
		if (snorlax == TBA) return;
		++i;
		while (i < d->dcb->fcb.size_in_blocks) {
			list[i] = dirblock->header.block_in_disk;
			if (dirblock->header.next_block != TBA) {
				snorlax = DiskDriver_readBlock(d->sfs->disk, dirblock, dirblock->header.next_block);
				if (snorlax == TBA) return;
			}
			++i;
		}
	}
	
	dirblock = NULL;
	free (dirblock);
	
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
					
					// List is going to contain all fileblocks
					int list[block->fcb.size_in_blocks];
					SimpleFS_remove_aux_file(f, list);
					
					// brutally deleting them				
					for (int j = 0; j < block->fcb.size_in_blocks; ++j) {						
						snorlax = DiskDriver_freeBlock(disk, list[j]);
						if (snorlax == TBA) return ERROR_FS_FAULT;
					}
					
					// updating the parent dir
					FirstDirectoryBlock* parent = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
					snorlax = DiskDriver_readBlock(disk, parent, f->fcb->fcb.directory_block);
					if (snorlax == TBA) return ERROR_FS_FAULT;
					parent->num_entries--;
					snorlax = DiskDriver_writeBlock(disk, parent, parent->header.block_in_disk);
					if (snorlax == TBA) return ERROR_FS_FAULT;
					
					parent = NULL;
					free (parent);
					
					f = NULL;
					free (f);
				}
				// or a DIR
				else {
					// creating a rudimental dirhandle
					DirectoryHandle* d = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));
					d->sfs = fs;
					d->dcb = (FirstDirectoryBlock*)block;
					d->directory = NULL;
					d->current_block = &block->header;
					d->pos_in_dir = 0;
					d->pos_in_block = 0;
					
					// List is going to contain all dirblocks
					int list[block->fcb.size_in_blocks];
					SimpleFS_remove_aux_dir(d, list);
					
					// reading the dir and storing all the names into dircontet
					int dirlen = d->dcb->num_entries;
					char* dircontent[dirlen];
					for (int j = 0; j < dirlen; ++j) {
						dircontent[j] = (char*) malloc(NAME_SIZE);
					}
					SimpleFS_readDir(dircontent, d);
					
					// for each name in the folder, recursively call the remove function on that name
					for (int j = 0; j < dirlen; ++j) {
						SimpleFS_remove(fs, dircontent[j]);
						
						//releasing memory
						dircontent[j] = NULL;
						free (dircontent[j]);
					}
					
					// brutally deleting all the directory blocks (stored in list)
					for (int j = 0; j < block->fcb.size_in_blocks; ++j) {
						snorlax = DiskDriver_freeBlock(disk, list[j]);
						if (snorlax == TBA) return ERROR_FS_FAULT;
					}
					
					// updating the parent dir
					FirstDirectoryBlock* parent = (FirstDirectoryBlock*) malloc(sizeof(FirstDirectoryBlock));
					snorlax = DiskDriver_readBlock(disk, parent, d->dcb->fcb.directory_block);
					if (snorlax == TBA) return ERROR_FS_FAULT;
					parent->num_entries -= 1;
					snorlax = DiskDriver_writeBlock(disk, parent, parent->header.block_in_disk);
					if (snorlax == TBA) return ERROR_FS_FAULT;
					
					parent = NULL;
					free (parent);					
					d = NULL;
					free (d);					
				}				
			}			
		}
				
	}
	
	block = NULL;
	free (block);
	return 0;
}
