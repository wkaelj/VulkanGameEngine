#include "debug.h"
#include <stdio.h>
#include <stdarg.h>


int debug_log (const char *format, ...) {

    return 0;

    // get list formats
    va_list args;
    va_start (args, format);
    int64_t requiredMem;

    requiredMem = vsnprintf (NULL, 0, format, args);
    if (requiredMem <= 0) {
        printf ("read failure\n");
        return EXIT_FAILURE;
    }

    // move va_list back to start
    va_end (args);
    va_start (args, format);

    putchar ('\0'); // malloc fails without this line idk why
    char *bufferString = (char *) malloc ((size_t) requiredMem);

    vsprintf (bufferString, format, args); 

    // print final string (I might change this to some in-game box later which is why I wrote this function)
    printf ("%s\n", bufferString);
    va_end (args);
    free (bufferString);
    return EXIT_SUCCESS;
}