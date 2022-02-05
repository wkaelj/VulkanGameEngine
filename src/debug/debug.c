#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


int debug_log (const char *format, ...) {

    // get list formats
    va_list args;
    va_start (args, format);
    int64_t requiredMem;

    requiredMem = vsnprintf (NULL, 0, format, args);
    if (requiredMem <= 0) {
        printf ("read failure\n");
        return EXIT_FAILURE;
    }
    // printf ("mem = %li\n", requiredMem);
    putchar ('\0'); // malloc fails without this line
    char *bufferString = malloc ((size_t) requiredMem);

    vsprintf (bufferString, format, args); 

    // print final string (I might change this to some in-game box later which is why I wrote this function)
    printf ("%s\n", bufferString);
    va_end (args);
    return EXIT_SUCCESS;
}