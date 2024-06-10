/*  <src/log.c>


*/


#include "log.h"


// Warning and errors
// ------------------

static unsigned int warning_count = 0;
static unsigned int error_count = 0;


inline void
_ContextWarningMsgf(const char * _ContextName, const char * _FormatMsg, ...)
{
    va_list arguments;
    va_start(arguments, _FormatMsg);

    es_fg_yellow();
    fprintf(stderr, "(#%2u) %s", ++ warning_count, _ContextName);
    aec_reset();

    vfprintf(stderr, _FormatMsg, arguments);
    fflush(stderr);

    va_end(arguments);
}

inline void
_ContextErrorMsgf(const char * _ContextName, const char * _FormatMsg, ...)
{
    va_list arguments;
    va_start(arguments, _FormatMsg);

    es_fg_red();
    fprintf(stderr, "(#%2u) %s", ++ error_count, _ContextName);
    aec_reset();

    vfprintf(stderr, _FormatMsg, arguments);
    fflush(stderr);

    va_end(arguments);
}

inline void _TracebackErrorArg(int argc, char ** argsv, int problematic_argument)
{
    aec_set_italic(); _TB_args_fg();
    for (int i = 0; i < argc; i ++) {
        if (i == problematic_argument) {
            aec_crs_save();
            _TB_hl_bg(); _TB_hl_fg();
            printf("%s", argsv[i]);
            aec_reset();
            putchar(' ');
            aec_set_italic(); _TB_args_fg();

        }
        else {
            printf("%s ", argsv[i]);
        }
    }////////

    aec_crs_left(1);
    aec_reset();
    putchar('\"');
    aec_crs_load();
    aec_crs_down(1);

    aec_reset();
    putchar('^');
}


#if IMPL_LOGGING

// Logging stream
// --------------

#if ! STDERR_DEBUG_LOGGING
static const char debug_filename[256] = STD_DEBUG_LOG_OUTPUT;
#endif

FILE * debug_stream = NULL;


bool InitializeLogging(void)
{
#if STDERR_DEBUG_LOGGING
    debug_stream = stderr;
#else
    debug_stream = fopen(debug_filename, "w");
    if (debug_stream == NULL)
        return false;
#endif
    return true;
}


void FinalizeLogging(void)
{
#if ! STDERR_DEBUG_LOGGING
    fclose(debug_stream);
#endif
}





inline bool __debug_true_inline_printf(const char * _FormatMsg, ...)
{
    va_list arguments;
    va_start(arguments, _FormatMsg);
    vfprintf(stdout, _FormatMsg, arguments);
    va_end(arguments);
    return true;
}






/*  */
static int db_level = 0;


static inline void __raiseDebugLevel(void)   { db_level ++; }
static inline void __fallDebugLevel(void)     {  if (db_level > 0) db_level --; }

inline void __printDebugSpacing(void)
{
    for (int i = 0; i < db_level; i++) {
        fputc('.', debug_stream);
        fputc('\t', debug_stream);
    }
}

inline void __debug_func_in(const char * _FunctionName)
{
    __printDebugSpacing();
    fprintf(debug_stream, "[%s] >>\n", _FunctionName);
    fflush(debug_stream);

    __raiseDebugLevel();
}

inline void __debug_func_out(const char * _FunctionName)
{
    __fallDebugLevel();
    __printDebugSpacing();

    fprintf(debug_stream, "[%s] <<\n", _FunctionName);
    fflush(debug_stream);
}


static void 
__debugprint_inner_db_level(void)
{
    static int __db_level = 0;
    if (db_level > __db_level)
        __db_level = db_level;

    while (__db_level != db_level) {
        __db_level--;
        fputc('\n', debug_stream);
    }
}


inline void
_DebugPrintf(const char * _FunctionName, uint8_t _Color, const char * _FormatMsg, ...)
{
    __debugprint_inner_db_level();

    // Initializing the variadic arguments list;
    va_list arguments;
    va_start(arguments, _FormatMsg);
    
    // setting the foreground color to blue and 
    // printing the function name;

    if (_Color == 0) {
        es_fg_blue();
    } else if (_Color == 1) {
        es_fg_yellow();
    } else if (_Color == 2) {
        es_fg_red();
    } else if (_Color == 3) {
		es_fg_green();
	}

    // tabbing
    __printDebugSpacing();

    // marking the function
    fprintf(debug_stream, "[%s] ", _FunctionName);
    aec_reset();
	
    // format printing the format-message with the variadic arguments.
    vfprintf(debug_stream, _FormatMsg, arguments);
    fflush(debug_stream);

    // de-allocating the variadic arguments list.
    va_end(arguments);
}

inline void
_DebugPrint(const char * _FunctionName, uint8_t _Color, const char * _Msg)
{
    __debugprint_inner_db_level();

    if (_Color == 0) {
        es_fg_blue();
    }
    else if (_Color == 1) {
        es_fg_yellow();
    }
    else if (_Color == 2) {
        es_fg_red();
    }
    else if (_Color == 3) {
        es_fg_green();
    }
    
    __printDebugSpacing();
    fprintf(debug_stream, "[%s] ", _FunctionName);
    aec_reset();

    fputs(_Msg, debug_stream);
    fflush(debug_stream);
}


#endif // IMPL_LOGGING
