# Operating Systems Project
Ref: https://sites.google.com/diag.uniroma1.it/sistemi-operativi-1819

****

   File System:
   implement a file system interface using binary files
   - The file system reserves the first part of the file
     to store:
     - a linked list of free blocks
     - linked lists of file blocks
     - a single global directory
     
   - Blocks are of two types
     - data blocks
     - directory blocks

     A data block is "random" information,
     A directory block contains a sequence of
     structs of type "directory_entry",
     containing the blocks where the files in that folder start
     and if they are directory themselves
     
Ref: https://gitlab.com/grisetti/sistemi_operativi_2018_19/tree/master/projects

****

## WHAT IS IT
The project Simple File System (SimpleFS) is a didactic project that focuses on the development of a basic File System.
It's composed by three levels - Bitmap, DiskDriver, SimpleFS - done to work together in a vertical layout to create and manage files and folders.


## HOW IT WORKS
As already specified, the perfect coordination between these three levels is essential to guarantee a good work of the software.

**Bitmap** :  the Bitmap is stored as an array of `uint8_t`. Each bit of each entry represents a single sotrage block of the disk and it's marked with 1 if the cell is occupied or 0 if the cell is free.
Because of this efficient representation of the bitmap it's mandatory operate with bit manipulation techniques when updating the bitmap.

**DiskDriver** : the DiskDriver is the middle level application that interfaces the SimpleFS with the Bitmap. Having an header with the proper informations and being able to see the bitmap, the DiskDriver simulates the driver of a Disk by mmapping the needed portion of memory in the central memory of the hosting machine.
Once initialized, the disk creates (or opens if the disk has been also initialized before) a file in where to copy the content of the mmapped memory to maintain persistance: at this level of reasoning, the file literally becomes a disk that contains all the stored informations. DiskDriver operates according to the mmapped memory and to the bitmap: by reading the bitmap it guarantees the correct operations of reading and writing on the mmapped memory, and so on the disk, giving response in case it's possible to write or read the requested block of the disk.

**SimpleFS** : the SimpleFS is the higher level software with which the user interacts. By communicating with the DiskDriver, the SimpleFS permits the creation an the manipulation of files and directories. 
It makes possible to create, write and remove files and directories in the limits of the disk's size.
The SimpleFS uses a Linked List system to stage datas from the disk: a folder is composed by multiple blocks in where to contain informations, so that files, too.
Each directory or file block contains an header with essential informations, such as the occupied block on the disk and the previous or the next block (if there are) in where the current block "expands".


## HOW TO RUN
To run the software is provided a test in *SO_FS/Tests*. Once there, launch the makefile with *make simplefs_test*.
After the compilation it's possible to run the software.
Looking at the output on the shell is possible to differentiate two other colors: red text inticates the choosen running option, yellow text gives information of what about the software is going to do.

### Running options
There are three running options that provide to test the File System initialization, the software behavior after the creation of a "large" number of files and its ongoing with the management of some directories.

- **File System initialization** : simply run *./simplefs_test* to initialize and, if needed, format the File System.
- **File Test** : run *./simplefs_test file* to initialize the FS and test file functions. This test creates a "large" number of files, opens one of them, writes and overwrites it and finally deletes the file.
- **Directory Test** : run *./simplefs_test dir* to initialize the FS and test directories functions. This test creates some files and some directories in the main folder and then deletes some of them.

- **Shell Mode** : run *./simplefs_test shell* to initialize the FS and test by your own using a simple provided shell. Type *help .* to show up all the shell commands.

### NOTES
This project is thought to satisfy the given specifications and to create a Simple File System with a single global directory and it's not for professional use.
