#include "fileutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

int readFileCharArray (char *filePath, char **ppCharacterOut,  uint32_t *pArrayLength) {

    // set array lenth to 0 so no break
    *pArrayLength = 0;

    // function variables
    char **ppOut; // store function output
    size_t fileSize = 0; // store file size

    FILE *file = fopen (filePath, "r");
    if (file == NULL) {
        debug_log ("Could not open file %s", filePath);
        return EXIT_FAILURE;
    }

    // find size of file
    fseek (file, 0L, SEEK_END); // go to end
    fileSize = ftell (file); // get position

    // throw error if file is 0 size
    if (fileSize == 0) {
        debug_log ("Cannot read file '%s' of size 0", filePath);
        return EXIT_FAILURE;
    }
    
    // go back to start of file
    fseek (file, 0L, SEEK_SET);

    // allocate enough memory to store whole file to ppOut
    ppOut = (char **) malloc (fileSize);

    // read all the lines in the file
    for (*pArrayLength = 0; fgets (ppOut[*pArrayLength], fileSize, file) == NULL; *pArrayLength++);
    
    for (int i = 0; i < 3; i++) {
        printf (ppOut[i]);
    }

    // set output
    ppCharacterOut = ppOut;

    return EXIT_SUCCESS;
}