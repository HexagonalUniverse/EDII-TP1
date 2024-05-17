/*
	Target: A way of making the "Print_Color" statements do not raising lift to processing
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
*/


#ifndef _ANSI_ESCAPE_CODE_HEADER_
#define _ANSI_ESCAPE_CODE_HEADER_


#include <stdio.h>		// For printing


// Main defintions.
#ifdef ANSI_ESCAPE_CODE
// --------------------


// AEC Control Sequence Introducer.
#define CSI		"\e["

#define _AEC_RESET			CSI "0m"	// AEC sequence for reseting the attributes.

// Resets all the attributes on the text output.
#define aec_reset()			printf(_AEC_RESET);


/*	Cursor and terminal screen sequences
	==================================== */

// Blink

#define _AEC_SLOW_BLINK		CSI "5m"	
#define _AEC_RAPID_BLINK	CSI "6m"
#define _AEC_BLINK_OFF		CSI	"25m"


// Cursor

#define _AEC_HOME			CSI "1"			// "Home" VT key.
#define _AEC_BEGINNING		"\e[H"			// "Move to line 1 column 1"

#define aec_beginning()		printf(_AEC_BEGINNING)

#define _AEC_CRS_UP(n)		CSI #n "A"
#define _AEC_CRS_DOWN(n)	CSI #n "B"
#define _AEC_CRS_RIGHT(n)	CSI #n "C"
#define _AEC_CRS_LEFT(n)	CSI #n "D"

#define aec_crs_up(n)		printf( _AEC_CRS_UP(n)	  )
#define aec_crs_down(n)		printf( _AEC_CRS_DOWN(n)  )
#define aec_crs_right(n)	printf( _AEC_CRS_RIGHT(n) )
#define aec_crs_left(n)		printf( _AEC_CRS_LEFT(n)  )

#define aec_crs_request()	printf(CSI "6n")	// Returns as "<CSI>#;#R".

#define aec_crs_save()		printf(CSI "s")
#define aec_crs_load()		printf(CSI "u")


#define aec_crslit_up(s)	printf(CSI s "A")
#define aec_crslit_down(s)	printf(CSI s "B")
#define aec_crslit_right(s)	printf(CSI "%s" "C", s)
#define aec_crslit_left(s)	printf(CSI s "D")


// Virtual Terminal

#define _AEC_INSERT			CSI "2"			// "Insert" VT key.
#define _AEC_BUFFER_SCREEN	_AEC_INSERT "J"

#define _AEC_DELETE			CSI "3"			// "Delete" VT key.
#define _AEC_ERASE_BUFFER	_AEC_DELETE "J"

#define _AEC_END			CSI "4"			// "Key" VT key.


#define _AEC_ERASE_CSR_END	CSI "0J"
#define _AEC_ERASE_CSR_BEG	CSI "1J"

#define _AEC_ERASE_CSR_END_LINE			CSI "0K"
#define _AEC_ERASE_BEG_CSRLINE_LINE		CSI	"1K"
#define _AEC_ERASE_LINE_LINE			CSI	"2K"


#define CSR_LINE_BEG					"\r"

#define aec_linebeg()	printf(CSR_LINE_BEG)


// AEC Sequence for cleaning the screen.
#define __CLEAN_SCREEN	_AEC_BEGINNING _AEC_BUFFER_SCREEN _AEC_ERASE_BUFFER


// Sequence for cleaning the (terminal) screen.
#define aec_clean()		printf(__CLEAN_SCREEN)


/*	Graphical sequences
	=================== */

// Default foreground coloring

#define _AEC_FG_BLACK	CSI "0;30m"
#define _AEC_FG_RED		CSI "0;31m"
#define _AEC_FG_GREEN	CSI "0;32m"
#define _AEC_FG_YELLOW	CSI "0;33m"
#define _AEC_FG_BLUE	CSI "0;34m"
#define _AEC_FG_PURPLE	CSI "0;35m"
#define _AEC_FG_CYAN	CSI	"0;36m"
#define _AEC_FG_WHITE	CSI "0;37m"


#define aec_fg_white()	printf(_AEC_FG_WHITE)
#define aec_fg_black()	printf(_AEC_FG_BLACK)
#define aec_fg_yellow()	printf(_AEC_FG_YELLOW)
#define aec_fg_red()	printf(_AEC_FG_RED)
#define aec_fg_blue()	printf(_AEC_FG_BLUE)
#define aec_fg_green()	printf(_AEC_FG_GREEN)
#define aec_fg_purple()	printf(_AEC_FG_PURPLE)
#define aec_fg_cyan()	printf(_AEC_FG_CYAN)


// Generic foreground and background coloring

#define _AEC_FG_PREFFIX	"\033[38;"
#define _AEC_BG_PREFFIX	"\033[48;"

#define aec_fg_rgb(r, g, b)	printf( _AEC_FG_PREFFIX "2;" #r ";" #g ";" #b "m" )
#define aec_bg_rgb(r, g, b)	printf( _AEC_BG_PREFFIX "2;" #r ";" #g ";" #b "m" )


// Text styles

#define aec_bold()
#define aec_unbold()




// Definitions boilerplate.
#else	// ANSI_ESCAPE_CODE


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