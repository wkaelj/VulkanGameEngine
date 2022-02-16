#include "fileutils.h"

#include <stdio.h>

int readFileBinary (char *filePath, unsigned char** pOut, size_t *pFileSize) {

    //open file
    FILE *file = fopen (filePath, "rb");
    size_t fileSizePool = 0; // declare here to avaid constant initializatoin
    unsigned char* buffer;

    // check if file opened
    if (file == NULL) { debug_log ("FILEUTILS: Could not open file: %s", filePath); return EXIT_FAILURE; }
    fseek (file, 0l, SEEK_END); // find end of files
    fileSizePool = ftell (file); // read bit at end of file aka size
    rewind(file);                // jump back to the beginning of the file

    // read file if more then 0 bytes
    if (fileSizePool == 0) { debug_log ("FILEUTILS: File has size 0: %s", filePath); return EXIT_FAILURE; }
    buffer = malloc (fileSizePool); // gice pOut mem same as size of file
    fread(buffer, fileSizePool, 1, file); // Read in the entire file

    // cleanup and return success
    fclose (file);
    *pFileSize = fileSizePool;
    *pOut = buffer;

    return EXIT_SUCCESS;
}

int readFileStringArray (char *filePath, char **ppCharacterOut,  uint32_t *pArrayLength) {

    // set array lenth to 0 so no break
    *pArrayLength = 0;

    // function variables
    FILE *file = fopen (filePath, "r");
    if (file == NULL) {
        debug_log ("Could not open file %s", filePath);
        return EXIT_FAILURE;
    }
    assert (file != NULL);


    // read number of linefeeds in file
    char bufferChar; // char buffer (awesome comment)
    size_t lineCount = 1; // store the total number of lines in the file to create output array (1 bc first line)
    while (1) {
        bufferChar = fgetc (file); // read char
        if (bufferChar == '\n') lineCount ++; // 
        if (bufferChar == EOF) break;
    }

    // return early if no char array output (very important to initialize character output)
    if (ppCharacterOut == NULL) {
        *pArrayLength = lineCount;
        return EXIT_SUCCESS;
    }
    assert (ppCharacterOut != NULL);

    // return to start of file
    fseek (file, 0l, SEEK_SET);

    // read lines and store in ppOut
    size_t lineStart; // store start of line so it can be returned
    size_t lineLength; // length of current line
    for (size_t i = 0; i < lineCount; i++) {

        lineLength = 0; // reset linelength

        lineStart = ftell (file); // store start of line
        while (1) {
            bufferChar = fgetc (file);
            if (bufferChar == '\n' || bufferChar == EOF) break; // break out of loop if char is a linefeed or end of file
            lineLength++; // if char is not linefeed or end of file increase linelength count by 1;
        }

        lineLength++; // make room for string terminator

        // return to start of line (stored up there)
        fseek (file, lineStart, SEEK_SET);

        // read full line of size lineLength
        ppCharacterOut[i] = malloc (lineLength * sizeof (char)); // allocate memory to string to store line variable
        if (fgets (ppCharacterOut[i], lineLength, file) == NULL) {
            debug_log ("Failed to read line %i from file %s", i, filePath);
            return EXIT_FAILURE;
        }

        // advance file path by 1 char in order to reach next line (skip over newline)
        fgetc (file);
    }

    // set output
    *pArrayLength = lineCount;

    return EXIT_SUCCESS;
}
