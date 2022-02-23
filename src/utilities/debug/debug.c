#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

// debug.h implementation
#define BUFFER_LENGTH 512
char bufferString1[BUFFER_LENGTH]; // bufferstring preallocated for stack storage
char bufferString2[BUFFER_LENGTH]; // I need 2

// debug_log function
int debug_log (DebugSeverity severity, const char *format, ...) {

    va_list args;
    va_start (args, format);

    // messege severity prefixes
    const char *messegeSeverity[] = {
        "[FATAL]: ",
        "[ERROR]: ",
	"[VALIDATION]: ",
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
        ">>> %s %s%s\n",
        ctime (&t),
        messegeSeverity[severity],
        bufferString1
    );

    // TODO fancy output
    printf ("%s", bufferString2);
    va_end (args);

    return SUCCESS;
    
}
