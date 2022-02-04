#include "fileutils.h"
#include <stdio.h>

size_t fileSizePool = 0; // declare here to avaid constant initializatoin
int readFileBinary (char *filePath, void *pOut) {

    // open file
    FILE *file = fopen (filePath, "rb");
    // check if file opened
    if (file == NULL) { debug_log ("FILEUTILS: Could not open file: %s", filePath); return EXIT_FAILURE; }
    fseek (file, 0L, SEEK_END); // find end of files
    fileSizePool = ftell (file); // read bit at end of file aka size

    // read file if more then 0 bytes
    if (fileSizePool == 0) { debug_log ("FILEUTILS: File has size 0: %s", filePath); return EXIT_FAILURE; }
    pOut = malloc (fileSizePool); // gice pOut mem same as size of file
    fread (pOut, sizeof (pOut), ftell (file), file);

    // cleanup and return success
    fclose (file);
    fileSizePool = 0;
    return EXIT_SUCCESS;
}