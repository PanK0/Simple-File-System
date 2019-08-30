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

     A data block is "random" information,`
     A directory block contains a sequence of
     structs of type "directory_entry",
     containing the blocks where the files in that folder start
     and if they are directory themselves
     
Ref: https://gitlab.com/grisetti/sistemi_operativi_2018_19/tree/master/projects

****

## WHAT IS IT
The project Simple File System (SimpleFS) is a didactic project that focuses on the development of a basic File System.
It's composed by three levels - Bitmap, DiskDriver, SimpleFS - done to work together in a vertical layout to create and manage files and folders.

## HOW
As already specified, the perfect coordination between these three levels is essential to guarantee a good work of the software.

**Bitmap** :  
