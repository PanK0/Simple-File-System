#include "simplefs_test_util.c"

#define NUM_BLOCKS 	500
#define NUM_FILES	400
#define BACK		".."

#define FILE_0	"Hell0"
#define FILE_1	"POt_aTO"
#define FILE_2	"MunneZ"
#define FILE_3	"cocumber"

#define DIR_0	"LOL"
#define DIR_1	"LEL"
#define DIR_2	"LUL"
#define DIR_3	"COMPITI"

#define BOLD_RED	 		"\033[1m\033[31m"
#define BOLD_YELLOW 		"\033[1m\033[33m"
#define RED     			"\x1b[31m"
#define YELLOW  			"\x1b[33m"
#define COLOR_RESET   		"\x1b[0m"

int main (int argc, char** argv) {

	// * * * * FILE SYSTEM INITIALIZATION * * * *
	
	printf (BOLD_RED "\n* * * * FILE SYSTEM INITIALIZATION * * * *\n"COLOR_RESET);
	
	// Init the disk and the file system
	printf (YELLOW "\n\n**	Initializing Disk and File System - testing SimpleFS_init()\n\n" COLOR_RESET);
	
	DiskDriver disk;
	DiskDriver_init(&disk, "simplefs_test.txt", NUM_BLOCKS);
	
	SimpleFS fs;
	DirectoryHandle* dirhandle;
	FileHandle* filehandle;
	dirhandle = SimpleFS_init(&fs, &disk);
	SimpleFS_print(&fs, dirhandle);
	
	if (dirhandle == NULL) {
		// Formatting the disk
		printf (YELLOW "\n\n**	Formatting File System - testing SimpleFS_format()\n\n" COLOR_RESET);
		SimpleFS_format(&fs);
		dirhandle = SimpleFS_init(&fs, &disk);
		SimpleFS_print(&fs, dirhandle);
	}
	
	// Giving current location
	printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
	printf ("\n");
	SimpleFS_printHandle(dirhandle);



	// * * * * TESTING FILES IF REQUESTED * * * *
	
	if (argc >= 2 && strcmp(argv[1], "file") == 0) {
		
		printf (BOLD_RED "\n* * * * TESTING FILES * * * *\n" COLOR_RESET); 
	
		// Printing the updated File System
		printf (YELLOW "\n\n** Printing the updated File System\n" COLOR_RESET);
		SimpleFS_print(&fs, dirhandle);
		
		// Giving Current Location
		printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
		SimpleFS_printHandle(dirhandle);
		SimpleFS_printDirBlocks(dirhandle);
		
		// Generating random strings and creating files
		// Using to test directory block capability limits
		printf (YELLOW "\n\n** Creating a lot of random files in the root dir\n" COLOR_RESET);
		
		char filenames[NUM_FILES][NAME_SIZE];
		for (int i = 0; i < NUM_FILES; ++i) {
			gen_filename(filenames[i], i);
			filehandle = SimpleFS_createFile(dirhandle, filenames[i]);
		}
		printf (YELLOW "** DONE\n" COLOR_RESET);
		
		// Printing the updated File System
		printf (YELLOW "\n\n** Printing the updated File System\n" COLOR_RESET);
		SimpleFS_print(&fs, dirhandle);
		
		// Giving current location
		printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
		SimpleFS_printHandle(dirhandle);
		
		// Printing the directory block list
		printf (YELLOW "\n\n** Printing the directory block list\n" COLOR_RESET);
		SimpleFS_printDirBlocks(dirhandle);
		
		// Reading a folder
		// Allocating an array
		printf (YELLOW "\n\n** Reading all files in directory %s - testing SimpleFS_readDir()\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		char* names[NUM_BLOCKS];
		for (int i = 0; i < NUM_BLOCKS; ++i) {
			names[i] = (char*) malloc(NAME_SIZE);
		}
		SimpleFS_readDir(names, dirhandle);
		printf (YELLOW "DONE\n" COLOR_RESET);
		
		// Listing files in the directory if requested
		if (argc >= 3 && strcmp(argv[2], "list") == 0) {
			printf (YELLOW "\n\n** Listing files in directory %s\n" COLOR_RESET, dirhandle->dcb->fcb.name);
			SimpleFS_printArray(names, NUM_BLOCKS);
		}
		
		// Opening a file
		printf (YELLOW "\n\n** Opening a file - testing SimpleFS_openFile()\n" COLOR_RESET);
		filehandle = SimpleFS_openFile(dirhandle, names[NUM_FILES/2]);
		SimpleFS_printHandle(filehandle);
		
		// Closing a file - DOES NOT WORK
		printf (YELLOW "\n\n** Closing a file - testing SimpleFS_close()\n" COLOR_RESET);
		//SimpleFS_close(filehandle);
		filehandle = NULL;
		SimpleFS_printHandle(filehandle);
		
		// Writing a file after re-opening it
		printf (YELLOW "\n\n** Writing a file - testing SimpleFS_write()\n" COLOR_RESET);
		
		char dante[] = "Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura ché la diritta via era smarrita.Ahi quanto a dir qual era è cosa dura esta selva selvaggia e aspra e forte che nel pensier rinova la paura! Tant'è amara che poco è più morte; ma per trattar del ben ch'i' vi trovai, dirò de l'altre cose ch'i' v'ho scorte. Io non so ben ridir com'i' v'intrai, tant'era pien di sonno a quel punto che la verace via abbandonai. Ma poi ch'i' fui al piè d'un colle giunto, là dove terminava quella valle che m'avea di paura il cor compunto, guardai in alto, e vidi le sue spalle vestite già de' raggi del pianeta che mena dritto altrui per ogne calle. Allor fu la paura un poco queta che nel lago del cor m'era durata la notte ch'i' passai con tanta pieta. E come quei che con lena affannata uscito fuor del pelago a la riva si volge a l'acqua perigliosa e guata, così l'animo mio, ch'ancor fuggiva, si volse a retro a rimirar lo passo che non lasciò già mai persona viva.";
		
		char omero[] = "Cantami, o Diva, l'ira funesta del pelide Achille che infiniti lutti addusse agli achei e gettò nell'Ade innumerevoli anime di eroi e abbandonò i loro corpi a cani e uccelli. Così si compì la volontà di Zeus, da quando al tempo indusse in contesa l'atride, re di popoli, e il divino Achille.";
		
		int sizedante = sizeof(dante) / sizeof(char);
		int sizeomero = sizeof(omero) / sizeof(char);
		
		filehandle = SimpleFS_openFile(dirhandle, names[NUM_FILES/2]);
		SimpleFS_printHandle(filehandle);
		
		int wdata = SimpleFS_write(filehandle, dante, sizedante);
		printf (YELLOW "\n\n** WRITTEN : %d BYTES. SHOULD BE : %d\n\n" COLOR_RESET, wdata, sizedante);
		SimpleFS_printHandle(filehandle);
		SimpleFS_printFileBlocks(filehandle);
		
		// Reading a file
		printf (YELLOW "\n\n** Reading file %s - testing SimpleFS_read()\n" COLOR_RESET, filehandle->fcb->fcb.name);
		char read_text[sizedante];
		int rdata = SimpleFS_read(filehandle, read_text, sizedante);
		printf (YELLOW "READ DATA : %d\n" COLOR_RESET, rdata);
		printf ("%s\n", (char*)read_text);
		
		// Seeking test
		filehandle = SimpleFS_openFile(dirhandle, names[NUM_FILES/2]);
		printf (YELLOW "\n\n** Seeking in file %s - Testing SimpleFS_seek()\n" COLOR_RESET, filehandle->fcb->fcb.name);
		int seeking_pos = 100;
		int pos = SimpleFS_seek(filehandle, seeking_pos);
		printf (YELLOW "CURSOR MOVED IN POS : %d, SHOULD BE : %d\n" COLOR_RESET, pos, seeking_pos);
		
		// Writing after seeking test
		printf (YELLOW "\n\n** Writing on file %s after seek - Testing SimpleFS_write()\n" COLOR_RESET, filehandle->fcb->fcb.name);
			// sizeomero-1 to avoid EOF
		wdata = SimpleFS_write(filehandle, omero, sizeomero-1);
		printf (YELLOW "WRITTEN : %d BYTES, SHOULD BE : %d\n" COLOR_RESET, wdata, sizeomero-1);
		SimpleFS_printHandle(filehandle);
		
		// Reading file again
		printf (YELLOW "\n\n** Reading file %s again - testing SimpleFS_read()\n" COLOR_RESET, filehandle->fcb->fcb.name);
		rdata = SimpleFS_read(filehandle, read_text, sizedante);
		printf (YELLOW "READ DATA : %d\n" COLOR_RESET, rdata);
		printf ("%s\n", (char*)read_text);
		
		// Removing a file and all it's content
		printf (YELLOW "\n\n** Removing file %s and all it's content - testing SimpleFS_remove()\n" COLOR_RESET, filehandle->fcb->fcb.name);
		SimpleFS_remove(&fs, filehandle->fcb->fcb.name);
		SimpleFS_printHandle(dirhandle);
		
		// Printing the updated File System
		printf (YELLOW "\n\n** Printing the updated File System\n" COLOR_RESET);
		SimpleFS_print(&fs, dirhandle);
		
		// Giving Current Location
		printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
		SimpleFS_printHandle(dirhandle);
		SimpleFS_printDirBlocks(dirhandle);
		
	}
	
	// * * * * TESTING DIRECTORIES IF REQUESTED * * * *
	
	if (argc >= 2 && strcmp(argv[1], "dir") == 0) {
		
		printf (BOLD_RED "\n* * * * TESTING DIRECTORIES * * * *\n" COLOR_RESET);
		
		// Printing the updated File System
		printf (YELLOW "\n\n** Printing the updated File System\n" COLOR_RESET);
		SimpleFS_print(&fs, dirhandle);
		
		// Giving Current Location
		printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
		SimpleFS_printHandle(dirhandle);
		SimpleFS_printDirBlocks(dirhandle);
		
		// Creating some files
		printf (YELLOW "\n\n** Creating some files - testing SimpleFS_createFile()\n" COLOR_RESET);
		
		printf (YELLOW "\n%s\n" COLOR_RESET, FILE_0);
		filehandle = SimpleFS_createFile(dirhandle, FILE_0);
		//SimpleFS_printHandle(filehandle);
		
		printf (YELLOW "%s\n" COLOR_RESET, FILE_1);
		filehandle = SimpleFS_createFile(dirhandle, FILE_1);
		//SimpleFS_printHandle(filehandle);
		
		printf (YELLOW "%s\n" COLOR_RESET, FILE_2);
		filehandle = SimpleFS_createFile(dirhandle, FILE_2);
		//SimpleFS_printHandle(filehandle);
		
		// Creating an already existent file
		printf (YELLOW "\n\n** Creating an already existent file - testing SimpleFS_createFile()\n" COLOR_RESET);
		printf (YELLOW "\n%s\n\n" COLOR_RESET, FILE_1);
		filehandle = SimpleFS_createFile(dirhandle, FILE_1);
		SimpleFS_printHandle(filehandle);
		
		// Creating some directories
		printf (YELLOW "\n\n** Creating some directories - testing SimpleFS_mkDir()\n" COLOR_RESET);
		
		printf (YELLOW "\n%s\n" COLOR_RESET, DIR_0);
		SimpleFS_mkDir(dirhandle, DIR_0);
// PROBLEMA : SE LANCIO PRIMA CON DIR E POI CON FILE QUANDO QUESTI DUE BLOCCHI SONO ATTIVI,
// FILE NON FUNZIONA E VA IN SEGFAULT		
	
		printf (YELLOW "%s\n" COLOR_RESET, DIR_1);
		SimpleFS_mkDir(dirhandle, DIR_1);
/*		
		printf (YELLOW "%s\n" COLOR_RESET, DIR_2);
		SimpleFS_mkDir(dirhandle, DIR_2);
*/		
		// Creating an already existent directory
		printf (YELLOW "\n\n** Creating an already existent directory - testing SimpleFS_mkDir()\n" COLOR_RESET);
		printf (YELLOW "\n%s\n" COLOR_RESET, DIR_0);
		SimpleFS_mkDir(dirhandle, DIR_0);
		
		// Reading a folder
		// Allocating an array
		printf (YELLOW "\n\n** Reading all files in directory %s - testing SimpleFS_readDir()\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		char* names[NUM_BLOCKS];
		for (int i = 0; i < NUM_BLOCKS; ++i) {
			names[i] = (char*) malloc(NAME_SIZE);
		}
		SimpleFS_readDir(names, dirhandle);
		printf (YELLOW "DONE\n" COLOR_RESET);
		
		// Listing files in the directory if requested
		if (argc >= 3 && strcmp(argv[2], "list") == 0) {
			printf (YELLOW "\n\n** Listing files in directory %s\n" COLOR_RESET, dirhandle->dcb->fcb.name);
			SimpleFS_printArray(names, dirhandle->dcb->num_entries);
		}
		
		// Changing directory
		printf (YELLOW "\n\n** Changing directory - testing SimpleFS_changeDir()\n" COLOR_RESET);
		SimpleFS_changeDir(dirhandle, DIR_0);
		SimpleFS_printHandle(dirhandle);
		SimpleFS_printDirBlocks(dirhandle);
		
		// Creating a new file into this directory
		printf (YELLOW "\n\n** Creating a file in %s - testing SimpleFS_createFile()\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		filehandle = SimpleFS_createFile(dirhandle, FILE_1);
		
// PROBLEMA : Continua ad creare in modo indiscriminato senza curarsi di duplicati.
/*		
		// Creating a new directory into this directory
		printf (YELLOW "\n\n** Creating a directory in %s - testing SimpleFS_mkDir()\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		SimpleFS_mkDir(dirhandle, DIR_3);
*/		
		// Reading a folder
		// Allocating an array
		printf (YELLOW "\n\n** Reading all files in directory %s - testing SimpleFS_readDir()\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		char* names2[NUM_BLOCKS];
		for (int i = 0; i < NUM_BLOCKS; ++i) {
			names2[i] = (char*) malloc(NAME_SIZE);
		}
		int a = SimpleFS_readDir(names2, dirhandle);
		printf (YELLOW "DONE %d\n" COLOR_RESET, a);
		
		// Listing files in the directory
		printf (YELLOW "\n\n** Listing files in directory %s\n" COLOR_RESET, dirhandle->dcb->fcb.name);
		SimpleFS_printArray(names2, dirhandle->dcb->num_entries);
		
		// Changing back directory
		printf (YELLOW "\n\n** Changing back directory - testing SimpleFS_changeDir()\n" COLOR_RESET);
		SimpleFS_changeDir(dirhandle, BACK);
		SimpleFS_printHandle(dirhandle);
		
		// Removing a dir and all it's content
		printf (YELLOW "\n\n** Removing dir %s and all it's content - testing SimpleFS_remove()\n" COLOR_RESET, DIR_0);
		SimpleFS_remove(&fs, DIR_0);
		SimpleFS_printHandle(dirhandle);		
		
		// Printing the updated File System
		printf (YELLOW "\n\n** Printing the updated File System\n" COLOR_RESET);
		SimpleFS_print(&fs, dirhandle);
		
		// Giving Current Location
		printf (YELLOW "\n\n** Giving current location\n" COLOR_RESET);
		SimpleFS_printHandle(dirhandle);

	}
	
	DiskDriver_flush(&disk);
	DiskDriver_unmap(&disk);
	close(disk.fd);
	
	return 0;
}
