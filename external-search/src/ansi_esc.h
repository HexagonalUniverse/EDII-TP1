/* <ansi_esc.h>

	Target: A way of making the "color printing" statements do not raise lift to processing
	if disabled during execution.

	References:
		Empty macros: https://itecnote.com/tecnote/c-empty-function-macros/;
		Colors range: https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences;
		Dealing with strings on preprocessors: https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation,
		https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html;

		Clean-screen explanation: https://stackoverflow.com/questions/66927511/what-does-e-do-what-does-e11h-e2j-do.

		https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797


	Issues:
		(Powershell not showing up colors by default, fix: https://superuser.com/questions/413073/windows-console-with-ansi-colors-handling/1300251#1300251)


	-----------------------------------------------------------------------------------------------------------------------------------------------

	Abbreviations:
		AEC: Ansi Escape Code.
		VT: Virtual terminal.
		NYI: Not yet implemented.

	
	last update: 25/5/24
*/


#ifndef _ANSI_ESCAPE_CODE_HEADER_
#define _ANSI_ESCAPE_CODE_HEADER_


// Main defintions.
#ifdef ANSI_ESCAPE_CODE
// --------------------

#include <stdio.h>		// For printing: _prints and printf.


#define _prints(s)		fputs(s, stdout)


// AEC control sequence introducer.
#define CSI					"\e["

// AEC sequence for reseting attributes.
#define _AEC_RESET			CSI "0m"	

// Resets all the attributes on the text output.
#define aec_reset()			_prints(_AEC_RESET)


// Cursor and terminal screen sequences
// ====================================

// Virtual Terminal
// ----------------

#define _AEC_INSERT			CSI "2"			// "Insert" VT key.
#define _AEC_BUFFER_SCREEN	_AEC_INSERT "J"

#define _AEC_DELETE			CSI "3"			// "Delete" VT key.
#define _AEC_ERASE_BUFFER	_AEC_DELETE "J"

#define _AEC_END			CSI "4"			// "Key" VT key.

#define _AEC_HOME			CSI "1"			// "Home" VT key.
#define _AEC_BEGINNING		"\e[H"			// "Move to line 1 column 1"


#define _AEC_ERASE_CSR_END	CSI "0J"
#define _AEC_ERASE_CSR_BEG	CSI "1J"

#define _AEC_ERASE_CSR_END_LINE			CSI "0K"
#define _AEC_ERASE_BEG_CSRLINE_LINE		CSI	"1K"
#define _AEC_ERASE_LINE_LINE			CSI	"2K"

#define CSR_LINE_BEG					'\r'

// AEC Sequence for cleaning the screen.
#define __CLEAN_SCREEN	_AEC_BEGINNING _AEC_BUFFER_SCREEN _AEC_ERASE_BUFFER

// Sequence for cleaning the (terminal) screen.
#define aec_clean()		printf(__CLEAN_SCREEN)


// Blinking state
// --------------

#define _AEC_SLOW_BLINK		CSI "5m"	
#define _AEC_RAPID_BLINK	CSI "6m"
#define _AEC_BLINK_OFF		CSI	"25m"


// Cursor positioning
// ------------------

// Moves the cursor to the beggining of the terminal screen (line 1, column 1)>
#define aec_beginning()		_prints(_AEC_BEGINNING)

// Moves the cursor to the beggining of it line on screen (to column 1).
#define aec_linebeg()		putc(CSR_LINE_BEG)

#define _AEC_CRS_UP(n)		CSI #n "A"
#define _AEC_CRS_DOWN(n)	CSI #n "B"
#define _AEC_CRS_RIGHT(n)	CSI #n "C"
#define _AEC_CRS_LEFT(n)	CSI #n "D"

/*	The position adjustment of the cursor by literal values takes
in consideration that it can be done directly without previous formatting.
For that, it should perhaps be more efficient, when that is the case. */

#define aec_crs_up_l(n)		_prints( _AEC_CRS_UP(n)    )	// Moves the cursor up by n positions. * n must be literal.
#define aec_crs_down_l(n)	_prints( _AEC_CRS_DOWN(n)  )	// Moves the cursor down by n positions. * n must be literal.
#define aec_crs_right_l(n)	_prints( _AEC_CRS_RIGHT(n) )	// Moves the cursor right by n positiions. * n must be literal.
#define aec_crs_left_l(n)	_prints( _AEC_CRS_LEFT(n)  )	// Moves the cursor left by n positions. * n must be literal.

#define aec_crs_up(n)		printf(	CSI "%dA", n )			// Moves the cursor up by n positions.
#define aec_crs_down(n)		printf(	CSI "%dB", n )			// Moves the cursor down by n positions.
#define aec_crs_right(n)	printf(	CSI "%dC", n )			// Moves the cursor right by n positions.
#define aec_crs_left(n)		printf(	CSI "%dD", n )			// Moves the cursor left by n positions.


// Cursor tracking
// ---------------

#define aec_crs_request()	_prints(CSI "6n")	// Returns as "<CSI>#;#R".

// Saves the current position of the cursor into a buffer.
#define aec_crs_save()		_prints(CSI "s")

// Loads a position from a previously saved on the buffer and moves the cursor to it.
#define aec_crs_load()		_prints(CSI "u")


// Graphical sequences
// ===================

/*	Default foreground coloring: The colors that are embedded in
most terminal systems. */

#define _AEC_FG_BLACK	CSI "0;30m"
#define _AEC_FG_RED		CSI "0;31m"
#define _AEC_FG_GREEN	CSI "0;32m"
#define _AEC_FG_YELLOW	CSI "0;33m"
#define _AEC_FG_BLUE	CSI "0;34m"
#define _AEC_FG_PURPLE	CSI "0;35m"
#define _AEC_FG_CYAN	CSI	"0;36m"
#define _AEC_FG_WHITE	CSI "0;37m"

#define aec_fg_white()	_prints(_AEC_FG_WHITE)
#define aec_fg_black()	_prints(_AEC_FG_BLACK)
#define aec_fg_yellow()	_prints(_AEC_FG_YELLOW)
#define aec_fg_red()	_prints(_AEC_FG_RED)
#define aec_fg_blue()	_prints(_AEC_FG_BLUE)
#define aec_fg_green()	_prints(_AEC_FG_GREEN)
#define aec_fg_purple()	_prints(_AEC_FG_PURPLE)
#define aec_fg_cyan()	_prints(_AEC_FG_CYAN)

/*	A more custom foreground and background coloring. Colors by specifing
the values for a RGB-color. */

#define _AEC_FG_PREFFIX	"\033[38;"
#define _AEC_BG_PREFFIX	"\033[48;"


#define _AEC_FG_RGB_L(r, g, b)	_AEC_FG_PREFFIX "2;" #r ";" #g ";" #b "m"
#define _AEC_BG_RGB_L(r, g, b)	_AEC_BG_PREFFIX "2;" #r ";" #g ";" #b "m"

#define aec_fg_rgb_l(r, g, b)	_prints( _AEC_FG_RGB_L(r, g, b)	)
#define aec_bg_rgb_l(r, g, b)	_prints( _AEC_BG_RGB_L(r, g, b) )


#define aec_fg_rgb(r, g, b)		printf( _AEC_FG_PREFFIX "2;%d;%d;%dm", r, g, b )
#define aec_bg_rgb(r, g, b)		printf( _AEC_BG_PREFFIX "2;%d;%d;%dm", r, g, b )


// Text styles
// -----------


#define _AEC_SET_ITALIC		CSI "3m"
#define aec_set_italic()	_prints( _AEC_SET_ITALIC )

#define _AEC_RESET_ITALIC	CSI "23m"
#define aec_reset_italic()	_prints( _AEC_RESET_ITALIC )


#define aec_bold()		// * NYI.
#define aec_unbold()	// * NYI.



#else	// Definitions boilerplate

#define aec_reset()
#define aec_beginning()

#define aec_crs_up(n)		
#define aec_crs_down(n)		
#define aec_crs_right(n)	
#define aec_crs_left(n)		

#define aec_crs_request()
#define aec_crs_save()		
#define aec_crs_load()	

#define aec_linebeg()	
#define aec_clean()		
#define aec_fg_white()	
#define aec_fg_black()	
#define aec_fg_yellow()	
#define aec_fg_red()	
#define aec_fg_blue()	
#define aec_fg_green()	
#define aec_fg_purple()	
#define aec_fg_cyan()	
#define aec_fg_rgb(r, g, b)
#define aec_bg_rgb(r, g, b)
#define aec_bold()
#define aec_unbold()


#endif	// ANSI_ESCAPE_CODE




#endif