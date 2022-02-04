#include "../dependencies.h"
#include "debug.h"
#include <stdio.h>
#include <stdarg.h>


size_t sizeofStringFormatChar (char format);

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

size_t sizeofStringFormatChar (char format) {
    size_t out;

    switch(format) {
        case 'c': return sizeof (char);
        case 's': return sizeof (char *);
        case 'i': return sizeof (int);
        case 'h': return sizeof (short);
        case 'l': return sizeof (long);
        case 'p': return sizeof (void *);
    };
    return 0;
}