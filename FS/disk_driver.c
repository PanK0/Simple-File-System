#include "disk_driver.h"

// opens the file (creating it if necessary_
// allocates the necessary space on the disk
// calculates how big the bitmap should be
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks) {
	
	int fok, fd;
	
	// Testing if the file exists (0) or not (-1)
	fok = access(filename, F_OK);
	
	// Getting the file descriptor
	fd = open(filename, O_CREAT | O_RDWR, 0666);
	if (fd == ERROR_FILE_OPENING) {
		printf ("ERROR : CANNOT OPEN THE FILE %s\n CLOSING . . .\n", filename);
		close(fd);
		exit(EXIT_FAILURE);
	}
	
	// Calculating dimensions for the map. I need space for:
	// the header -> sizeof(DiskHeader)
	// the bitmap entries array -> num_blocks/NUMBITS 		NUMBITS = 8
	// blocks -> num_blocks * BLOCK_SIZE					BLOCK_SIZE = 512
	size_t header_dim	= sizeof(DiskHeader);
	size_t entries_dim	= num_blocks / NUMBITS;
	size_t blocklist_dim	= num_blocks * BLOCK_SIZE;
	size_t map_dim		= header_dim + entries_dim + blocklist_dim;
	
	// Mapping the space I need. Choosing this attributes:
	// NULL : I let the kernel choose the best position for the map
	// PROT_READ | PROT_WRITE : operations to do with the file. Don't need to execute
	// MAP_SHARED : not private because if so, I could not modify the "disk" with "persistance"
	void* mapped_mem = mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	// Starting to set up my Disk Driver
	disk->header = (DiskHeader*) mapped_mem;
	disk->bitmap_data = (uint8_t*) mapped_mem + header_dim;
	disk->fd = fd;	
	disk->header->num_blocks = num_blocks;
	disk->header->bitmap_blocks = num_blocks;
	disk->header->bitmap_entries = entries_dim;
	
	
	// IF the file was already existent I just need to do operations on free blocks
	// ELSE I need to set the entire bitmap on zero
	if (fok == 0) {		
		BitMap bmap;
		bmap.num_bits = entries_dim;
		bmap.entries = disk->bitmap_data;		
		
		disk->header->free_blocks = BitMap_getFreeBlocks(&bmap);
		disk->header->first_free_block = BitMap_get(&bmap, 0, FREE);		
	}
	else {
		for (int i = 0; i < entries_dim; ++i) {
			(disk->bitmap_data)[i] = 0;
		}
		disk->header->free_blocks = num_blocks;
		disk->header->first_free_block = 0;
	}
}

// reads the block in position block_num
// returns -1 if the block is free according to the bitmap
// 0 otherwise
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num) {
	
	// Calculating the offset where the blocklist starts (in the file)
	// and positioning a file pointer where I need to read the block (block_num * BLOCK_SIZE)
	off_t blocklist_start = (off_t) sizeof(DiskHeader) + disk->header->bitmap_entries;
	int pellegrino = lseek(disk->fd, blocklist_start + block_num * BLOCK_SIZE, SEEK_SET);
	if (pellegrino == ERROR_FILE_SEEKING) {
		printf ("ERROR : CANNOT POSITION FILE POINTER\n CLOSING . . .\n");
		exit(EXIT_FAILURE);
	}
	
	// Reading that block and putting it into dest
	pellegrino = read(disk->fd, dest, BLOCK_SIZE);
	if (pellegrino == ERROR_FILE_READING) {
		printf ("ERROR : CANNOT READ THE FILE\n CLOSING . . .\n");
		exit(EXIT_FAILURE);
	}
	
	BitMap bmap;
	bmap.num_bits = disk->header->num_blocks;
	bmap.entries = disk->bitmap_data;
	
	int isSet = BitMap_isBitSet(&bmap, block_num);
	if (isSet) return 0;
	else return -1;
}

// writes a block in position block_num, and alters the bitmap accordingly
// returns the number of written blocks if success
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num) {
	
	// Calculating the offset where the blocklist starts (in the file)
	// and positioning a file pointer where I need to read the block (block_num * BLOCK_SIZE)
	off_t blocklist_start = (off_t) sizeof(DiskHeader) + disk->header->bitmap_entries;
	int pellegrino = lseek(disk->fd, blocklist_start + block_num * BLOCK_SIZE, SEEK_SET);
	if (pellegrino == ERROR_FILE_SEEKING) {
		printf ("ERROR : CANNOT POSITION FILE POINTER\n CLOSING . . .\n");
		exit(EXIT_FAILURE);
	}
	
	// Writing src in that block
	pellegrino = write(disk->fd, src, BLOCK_SIZE);
	if (pellegrino == ERROR_FILE_WRITING) {
		return ERROR_FILE_WRITING;
	}
	
	// Altering the bitmap and updating the DiskHeader
	BitMap bmap;
	bmap.num_bits = disk->header->num_blocks;
	bmap.entries = disk->bitmap_data;
	int set = BitMap_set(&bmap, block_num, OCCUPIED);
	if (set == ERROR_RESEARCH_FAULT) {
		printf ("ERROR : CANNOT LOOK FOR THE WANTED BIT DURING WRITING\n CLOSING . . .\n");
		exit(EXIT_FAILURE);
	}
	--(disk->header->free_blocks);
	disk->header->first_free_block = BitMap_get(&bmap, 0, FREE);
	
	return pellegrino;
}

// frees a block in position block_num, and alters the bitmap accordingly
// don't need to write all zeroes in the memory: just change the bitmap.
// returns -1 if operation not possible, 0 if success
int DiskDriver_freeBlock(DiskDriver* disk, int block_num) {
	BitMap bmap;
	bmap.num_bits = disk->header->num_blocks;
	bmap.entries = disk->bitmap_data;
	int set = BitMap_set(&bmap, block_num, FREE);
	if (set == ERROR_RESEARCH_FAULT) {
		return ERROR_RESEARCH_FAULT;
	}
	
	// Updating the DiskHeader
	++(disk->header->free_blocks);
	disk->header->first_free_block = BitMap_get(&bmap, 0, FREE);
	
	return set;
}

// returns the first free blockin the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start) {
	BitMap bmap;
	bmap.num_bits = disk->header->num_blocks;
	bmap.entries = disk->bitmap_data;
	return BitMap_get(&bmap, start, FREE);
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk) {
	
	int num_blocks = disk->header->num_blocks;
	int pellegrino;
	
	// Calculating map dimensions
	size_t header_dim = sizeof(DiskHeader);
	size_t entries_dim = num_blocks / NUMBITS;
	size_t blocklist_dim = num_blocks * BLOCK_SIZE;
	size_t map_dim = header_dim + entries_dim + blocklist_dim;
	
	// Flushing the map
	pellegrino = msync(disk->header, map_dim, MS_SYNC);
	if (pellegrino != 0) {
		printf ("ERROR : CANNOT FLUSH THE MAP\n CLOSING . . .\n");
		exit(EXIT_FAILURE);
	}
	return pellegrino;
}
