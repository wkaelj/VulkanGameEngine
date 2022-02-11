#ifndef FILE_UTILITIES
#define FILE_UTILITIES

#include "../debug/debug.h"

// reads a file as binary, stores it in void pointer
int readFileBinary (char *filePath, void *pOut, size_t *pFileSize);

// reads a file and breaks it up at each linefeed. Retuns as a array of char arrays
int readFileStringArray (char *filePath, char **ppCharacterOut, u_int32_t *pArrayLength);

#endif