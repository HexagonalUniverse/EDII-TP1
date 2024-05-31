/*  <src/log.h>

    Definition of the logging system for the implementations. */


#ifndef _ES_LOG_HEADER_
#define _ES_LOG_HEADER_


#define ANSI_ESCAPE_CODE    true
#include "ansi_esc.h"
#include <stdarg.h>
#include <stdbool.h>


// Standard Colors
// ---------------

#define es_fg_blue()     aec_fg_rgb_l(75, 135, 225)    // Switchs the foreground to blue.

#define _ES_FG_RED()          _AEC_FG_RGB_L(175, 75, 75)
#define es_fg_red()     aec_fg_rgb_l(175, 75, 75)     // Switchs the foreground to red.

#define es_fg_yellow()       aec_fg_rgb_l(225, 225, 75)        // Switchs the foreground to yellow.
#define es_fg_green()	    aec_fg_rgb_l(75, 225, 75)         // Switchs the foreground to green.


// Traceback colors
// ----------------

#define _TB_args_fg()       aec_fg_rgb_l(175, 175, 200)
#define _TB_hl_fg()         aec_fg_rgb_l(225, 160, 180)   
#define _TB_hl_bg()         aec_bg_rgb_l(85, 25, 40)


// Warning and errors
// ------------------

void _ContextWarningMsgf(const char * _ContextName, const char * _FormatMsg, ...);
void _ContextErrorMsgf(const char * _ContextName, const char * _FormatMsg, ...);

#define WarningCF(_FormatMsg, ...)       _ContextWarningMsgf(__func__, _FormatMsg, __VA_ARGS__)
#define ErrorCF(_FormatMsg, ...)         _ContextErrorMsgf(__func__, _FormatMsg, __VA_ARGS__)


void _TracebackErrorArg(int argc, char ** argsv, int problematic_argument);




// #define IMPL_LOGGING              true


#if IMPL_LOGGING
#include <inttypes.h>

// Initialization and finalization
// -------------------------------

bool InitializeLogging(void);
void FinalizeLogging(void);


// Logging stream
// --------------

#define STD_DEBUG_LOG_OUTPUT  "temp/_last_debug.log"

#ifndef STDERR_DEBUG_LOGGING
#define STDERR_DEBUG_LOGGING      false
#endif // STDERR_DEBUG_LOGGING




// Debugging
// ---------

// Debugging directives

#define DEBUG_PAGE_READING                  false
#define DEBUG_PAGE_WRITING                  true
#define DEBUG_REG_INDEX_IN_BUILD            true
#define DEBUG_STREAM_AFTER_INSERTION        true
#define DEBUG_FRAME_PAGE_MANAGEMENT         true    // update and retrieving
#define DEBUG_FRAME_REFRESH                 true





bool __debug_true_inline_printf(const char * _FormatMsg, ...);

void __debug_func_in(const char * _FunctionName);
void __debug_func_out(const char * _FunctionName);
void __printDebugSpacing(void);


#define fallDebug()         __debug_func_out(__func__)
#define raiseDebug()        __debug_func_in(__func__)
#define printDebugSpacing() __printDebugSpacing()

extern FILE * debug_stream;


// below old

void _DebugPrintf(const char * _FunctionName, uint8_t _Color, const char * _FormatMsg, ...);
void _DebugPrint(const char * _FunctionName, uint8_t _Color, const char * _Msg);


#define DebugPrint(_FormatMsg)          _DebugPrint(__func__, 0, _FormatMsg)

/*  Debugs a function execution, on stderr, with a format message. 
    In that sense, it's used as printf inside a function context. */
#define DebugPrintf(_FormatMsg, ...)     _DebugPrintf(__func__, 0, _FormatMsg, __VA_ARGS__)

#define DebugPrintY(_FormatMsg, ...)    _DebugPrintf(__func__, 1, _FormatMsg, __VA_ARGS__)

#define DebugPrintR(_FormatMsg, ...)    _DebugPrintf(__func__, 2, _FormatMsg, __VA_ARGS__)

#define DebugPrintG(_FormatMsg, ...)    _DebugPrintf(__func__, 3, _FormatMsg, __VA_ARGS__)


// The DebugPrintf but without the message.
#define DebugFuncMark()                 DebugPrintf(NULL, NULL); fputc('\n', stderr);












#endif // IMPL_LOGGING


#endif // _ES_LOG_HEADER_
