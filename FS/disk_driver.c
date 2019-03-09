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
	
	// Getting a file descriptor
	int fd = open(filename, O_RDWR | O_CREAT);
	if (fd = ERROR_FILE_OPENING) {
		printf ("ERROR : CANNOT OPEN THE FILE %s\n CLOSING . . .\n", filename);
		exit(EXIT_FAILURE);
	}
	
	// Calculating dimensions for the map. I need space for:
	// the header -> sizeof(DiskHeader)
	// the bitmap entries array -> num_blocks/NUMBITS 		NUMBITS = 8
	// blocks -> num_blocks * BLOCK_SIZE					BLOCK_SIZE = 512
	size_t header_dim	= sizeof(DiskHeader);
	size_t entries_dim	= num_blocks / NUMBITS;
	size_t blocklist_dim	= num_blocks * BLOCK_SIZE;
	size_t map_dim		= header_dim + entrie_dim + blocklist_dim;
	
	// Mapping the space I need. Choosing this attributes;
	// NULL : I let the kernel choose the best position for the map
	// PROT_READ | PROT_WRITE : operations to do with the file. Don't need to execute
	// MAP_SHARED : not private because if so, I could not modify the "disk" with "persistance"
	void* mapped_mem = mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if ( (int) mapped_mem == ERROR_MAP_FAILED) {
		printf ("ERROR : CANNOT MAP THE FILE %s\n CLOSING . . .\n", filename);
		exit(EXIT_FAILURE);
	}
	
	// Starting to set my Disk Driver
	disk->header = (DiskHeader*) mapped_mem + 0;
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
