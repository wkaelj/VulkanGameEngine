#ifndef FILE_UTILITIES
#define FILE_UTILITIES

#include "../dependencies.h"

// reads a file as binary, stores it in void pointer
int readFileBinary (char *filePath, void *pOut);

// reads a file and breaks it up at each linefeed. Retuns as a array of char arrays
int readFileCharArray (char *filePath, char **ppCharacterOut, uint32_t *pArrayLength);

#endif