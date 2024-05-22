#include "log.h"


static int tabs = 0;

// -> _raiseDebugLevel.
inline void _startDebug(void)   { tabs ++; }
inline void _endDebug(void)     { if (tabs > 0) tabs --; }


inline void
_DebugPrint(const char * _FunctionName, size_t _Color, const char * _FormatMsg, ...)
{
    static int __tabs = 0;
    if (__tabs != tabs) {
        putchar('\n');
        __tabs = tabs;
    }

    // Initializing the variadic arguments list;
    va_list arguments;
    va_start(arguments, _FormatMsg);
    
    // setting the foreground color to blue and 
    // printing the function name;

    if (_Color == 0) {
        _Blue();
    } else if (_Color == 1) {
        _Yellow();
    } else if (_Color == 2) {
        _Red();
    } else if (_Color == 3) {
		_Green();
	}

    // tabbing
    for (int i = 0; i < tabs; i++) {
        putchar('.');
        putchar('\t');
    }

    // marking the function
    fprintf(stderr, "[%s] ", _FunctionName);
    aec_reset();
	
    // format printing the format-message with the variadic arguments.
    vfprintf(stderr, _FormatMsg, arguments);
    fflush(stderr);

    // de-allocating the variadic arguments list.
    va_end(arguments);
}
