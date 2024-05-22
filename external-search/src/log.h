// <log.h>


#ifndef _LOG_HEADER_
#define _LOG_HEADER_


#define ANSI_ESCAPE_CODE    true
#include "ansi_esc.h"
#include <stdarg.h>


// Debugging
// ---------

#define _Blue()     aec_fg_rgb(75, 135, 225)
#define _Red()      aec_fg_rgb(225, 75, 75)
#define _Yellow()   aec_fg_rgb(225, 225, 75)
#define _Green()	aec_fg_rgb(75, 255, 75)


void _DebugPrint(const char * _FunctionName, size_t _Color, const char * _FormatMsg, ...);


/*  Debugs a function execution, on stderr, with a format message. 
    In that sense, it should be used as printf inside a function context. */
#define DebugPrint(_FormatMsg, ...)     _DebugPrint(__func__, 0, _FormatMsg, __VA_ARGS__)

#define DebugPrintY(_FormatMsg, ...)    _DebugPrint(__func__, 1, _FormatMsg, __VA_ARGS__)

#define DebugPrintR(_FormatMsg, ...)    _DebugPrint(__func__, 2, _FormatMsg, __VA_ARGS__)

#define DebugPrintG(_FormatMsg, ...)    _DebugPrint(__func__, 3, _FormatMsg, __VA_ARGS__)


// The DebugPrint but without the message.
#define DebugFuncMark()                 DebugPrint(NULL, NULL); fputc('\n', stderr);


void _startDebug(void);
void _endDebug(void);
#define endDebug()          _endDebug()
#define startDebug()        _startDebug()


// Debug directives

#define DEBUG_READ_EBSTNODE     false
#define DEBUG_WRITE_EBSTNODE    false

#endif
