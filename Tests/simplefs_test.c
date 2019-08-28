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
	
	char text[] = "Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura ché la diritta via era smarrita.Ahi quanto a dir qual era è cosa dura esta selva selvaggia e aspra e forte che nel pensier rinova la paura! Tant'è amara che poco è più morte; ma per trattar del ben ch'i' vi trovai, dirò de l'altre cose ch'i' v'ho scorte. Io non so ben ridir com'i' v'intrai, tant'era pien di sonno a quel punto che la verace via abbandonai. Ma poi ch'i' fui al piè d'un colle giunto, là dove terminava quella valle che m'avea di paura il cor compunto, guardai in alto, e vidi le sue spalle vestite già de' raggi del pianeta che mena dritto altrui per ogne calle. Allor fu la paura un poco queta che nel lago del cor m'era durata la notte ch'i' passai con tanta pieta. E come quei che con lena affannata uscito fuor del pelago a la riva si volge a l'acqua perigliosa e guata, così l'animo mio, ch'ancor fuggiva, si volse a retro a rimirar lo passo che non lasciò già mai persona viva.";
	
	char text2[] = "Cantami, o Diva, l'ira funesta del pelide Achille che infiniti lutti addusse agli achei e gettò nell'Ade innumerevoli anime di eroi e abbandonò i loro corpi a cani e uccelli. Così si compì la volontà di Zeus, da quando al tempo indusse in contesa l'atride, re di popoli, e il divino Achille.";
	
	int size = sizeof(text) / sizeof(char);
	int size2 = sizeof(text2) / sizeof(char);
	
	int wdata = SimpleFS_write(filehandle, text, size);
	printf ("Written : %d bytes. Should be : %d\n", wdata, size);
	SimpleFS_printHandle(filehandle);
	SimpleFS_printFileBlocks(filehandle);
	
	// Reading a file
	printf ("\n**	Reading a file - testing SimpleFS_read() \n");
	char read_text[size]; // = (void*)malloc(sizeof(char) * size);
	int rdata = SimpleFS_read(filehandle, read_text, size);
	//printf ("%s\n", (char*)read_text);
	SimpleFS_printFileBlocks(filehandle);


	filehandle = SimpleFS_openFile(dirhandle, names[NUM_FILES/2]);
	
	printf ("\n**	Seeking in a file - testing SimpleFS_seek() \n");
	int seeking_pos = 100;
	int pos = SimpleFS_seek(filehandle, seeking_pos);
	printf ("Cursor moved on pos : %d, should be : %d\n", pos, seeking_pos);
	SimpleFS_printHandle(filehandle);
	
	printf ("\n**	Writing on the same file after seek - testing SimpleFS_write() \n");
	wdata = SimpleFS_write(filehandle, text2, size2-1);
	printf ("\nWritten : %d bytes. Should be : %d\n", wdata, size2-1);
	SimpleFS_printHandle(filehandle);
	
	SimpleFS_printFileBlocks(filehandle);

	// Reading a file
	printf ("\n**	Reading a file - testing SimpleFS_read() \n");
	rdata = SimpleFS_read(filehandle, read_text, size);
	printf ("%s\n", (char*)read_text);
	
	printf ("Read data : %d, should be: %d\n", rdata, size);
	
	// Creating a new directory
	printf ("\n**	Creating a new directory - testing SimpleFS_mkDir() \n");
	SimpleFS_mkDir(dirhandle, "LOL");
	SimpleFS_printHandle(dirhandle);

	
	// Changing directory
	printf ("\n**	Changing directory - testing SimpleFS_changeDir() \n");
	SimpleFS_changeDir(dirhandle, "LOL");
	SimpleFS_printHandle(dirhandle);
	
	// Changing directory
	printf ("\n**	Changing directory - testing SimpleFS_changeDir() \n");
	SimpleFS_changeDir(dirhandle, "..");
	SimpleFS_printHandle(dirhandle);
	
	// Creating a new directory
	printf ("\n**	Creating a new directory - testing SimpleFS_mkDir() \n");
	SimpleFS_mkDir(dirhandle, "LUL");
	SimpleFS_printHandle(dirhandle);
	
	// Changing directory
	printf ("\n**	Changing directory - testing SimpleFS_changeDir() \n");
	SimpleFS_changeDir(dirhandle, "LUL");
	SimpleFS_printHandle(dirhandle);

	// Removing a file
	printf ("\n**	Removing a file - testing SimpleFS_remove() \n");
	SimpleFS_remove(&fs, "File_200");
	
	SimpleFS_print(&fs, dirhandle);
	
	// Creating an non-existent file
	printf ("\n**	Creating a non-existent file - testing SimpleFS_createFile() \n");
	filehandle = SimpleFS_createFile(dirhandle, FILE_2);
	SimpleFS_printHandle(filehandle);
	
	DiskDriver_flush(&disk);
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
