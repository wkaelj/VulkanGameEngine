#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


#define BUFFER_LENGTH 512
char bufferString1[BUFFER_LENGTH]; // bufferstring preallocated for stack storage
char bufferString2[BUFFER_LENGTH]; // I need 2
int debug_log (DebugSeverity severity, const char *format, ...) {

    va_list args;
    va_start (args, format);

    // messege severity prefixes
    const char *messegeSeverity[] = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[INFO]: ",
        "[DEBUG]: "
    };

    // get time
    time_t t;
    time(&t);

    // pupulate bufferstring 1 with messege and args and all that
    vsnprintf (bufferString1, BUFFER_LENGTH, format, args);

    snprintf (
        bufferString2,
        BUFFER_LENGTH,
        "%s %s%s",
        ctime (&t),
        messegeSeverity[severity],
        bufferString1
    );

    // TODO fancy output

    va_end (args);
    
}