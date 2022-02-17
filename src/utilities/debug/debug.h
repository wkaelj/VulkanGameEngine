#ifndef DEBUG_H
#define DEBUG_H

#include <sve_header.h>


/**
 * @brief Functions to output and log debug messeges.
 * @author Kael Johnston
 * @date Feb 17 2022
 */


// should enable 
#define ENABLE_FATAL 1
#define ENABLE_ERROR 1
#define ENABLE_INFO 1
#define ENABLE_DEBUG 1

typedef enum {
    DEBUG_FATAL = 0,
    DEBUG_ERROR = 1,
    DEBUG_INFO = 2,
    DEBUG_DEBUG = 3,
} DebugSeverity;

// check if debug messeges are enabled, otherwise disable the corrospining messege
#ifdef ENABLE_FATAL
#define LOG_FATAL (messege, ...) debug_log (DEBUG_FATAL, messege, __VA_ARGS__)
#else
#define LOG_FATAL (messege, ...)
#endif

#ifdef ENABLE_ERROR
#define LOG_ERROR (messege, ...) debug_log (DEBUG_ERROR, messege, __VA_ARGS__)
#else
#define LOG_ERROR (messege, ...)
#endif

#ifdef ENABLE_ERROR
#define LOG_INFO (messege, ...) debug_log (DEBUG_INFO, messege, __VA_ARGS__)
#else
#define LOG_INFO (messege, ...)
#endif

#ifdef ENABLE_DEBUG
#define LOG_DEBUG (messege, ...) debug_log (DEBUG_DEBUG, messege, __VA_ARGS__)
#else
#define LOG_DEBUG (messege, ...)
#endif

// log debugging messege
int debug_log (DebugSeverity severity, const char *format, ...);

#endif