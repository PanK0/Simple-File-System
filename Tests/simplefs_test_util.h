#pragma once
#include "../FS/simplefs.c"
#include <stdlib.h>

// Prints a FirstDirectoryBlock
void SimpleFS_printFirstDir(SimpleFS* fs, DirectoryHandle* d);

// Prints the Disk Driver content
void SimpleFS_print (SimpleFS* fs, DirectoryHandle* d);

// Prints the current directory location
void SimpleFS_printHandle (void* h);

// Generates a random string
void gen_random (char *s, const int len);

// Generates names for files
void gen_filename(char *s, const int len);
