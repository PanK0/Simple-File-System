#pragma once
#include "../FS/simplefs.c"

// Prints a FirstDirectoryBlock
void SimpleFS_printFirstDir(SimpleFS* fs, FirstDirectoryBlock* d);

// Prints the Disk Driver content
void SimpleFS_print (SimpleFS* fs);

// Prints the current directory location
void SimpleFS_printHandle (void* h);
