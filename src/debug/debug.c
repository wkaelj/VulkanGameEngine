#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


int debug_log (const char *format, ...) {

    #define RESET_ARGS va_end(args); va_start(args, format); // macro to reset args to start

    #ifdef DISABLE_DEBUG_FUNC // run simplified debug code for testing purposes
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    #else

    // this code can be disabled to test other parts of the program. It corrputs memory

    // variable decleration
    char *output;
    size_t stringSize;
    va_list args;
    va_start (args, format);

    // get size of string and allocate mem for output
    stringSize = vsnprintf (NULL, 0, format, args);
    output = malloc (stringSize);

    RESET_ARGS;

    // print format string to output variable
    vsprintf (output, format, args);

    // print output string to stdout
    printf ("%s\n", output);
    free (output);
    return EXIT_SUCCESS;

    #endif
}