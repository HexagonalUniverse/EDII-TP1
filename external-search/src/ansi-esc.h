/* <ansi_esc.h>		by	<hexadic-tools/src>

	An API for Ansi Escape Code.

	Side-effects escaping disabled by default. To toggle it, do define ANSI_ESCAPE_CODE before including the header file.

	Target: Besides the API, a way of making the escaping code call do not raise any lift to
	processing if disabled, while the procedures are still being referenced to. (In that sense,
	coloring the foreground to the default red color, while it is disabled, equals an unaction,
	at the step that having it enabled will print to stdout the sequence.)

	Last update: 09/6/24. */

	/* ------------------------------------------------------------

		Abbreviations:
			AEC: Ansi Escape Code.
			VT: Virtual terminal.
			NYI: Not yet implemented.

		TODO's:
			. Choose output stream, rather than default stdout?


		References:
			Empty macros: https://itecnote.com/tecnote/c-empty-function-macros/;
			Colors range: https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences;
			Dealing with strings on preprocessors: https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation,
			https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html;
			Clean - screen explanation: https://stackoverflow.com/questions/66927511/what-does-e-do-what-does-e11h-e2j-do;
			Various codes table: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797.

		Issues:
		(Powershell not showing up colors by default, fix: https://superuser.com/questions/413073/windows-console-with-ansi-colors-handling/1300251#1300251)

	------------------------------------------------------------ */


#ifndef _ANSI_ESCAPE_CODE_HEADER_
#define _ANSI_ESCAPE_CODE_HEADER_


	/*
		API Conventions:
		===============

		def: ANSI-ESC 'sequences' are character sequences that represents a code in ANSI-ESC standard.
		Those functions are the behavior they invoke.

		Macros starting with an underline "_" tells the intention of privating it; they are only
		intended to be used internally.

		ANSI-ESC sequences starts with <_AEC_> and are upper case, as in <_AEC_RESET> for "\e[0m".
		They start with a single underline in case they're a more direct translation of the codes,
		and without in case they're an abstraction. For example, <_AEC_INSERT> and <AEC_CLEAN_SCREEN>.

		ANSI-ESC functions are lowercase, starts with <aec_>, and are always callables, as in aec_reset(),
		for resambling side-effect it provoke.

		In aec functions that takes a numerical parameter, the version of them that terminates with "_l" are
		intended to be used when the value is literal. For instance, if one wants to colors the foreground in
		pure red, rgb <255, 0, 0>, it should be used "aec_fg_rgb_l(255, 0, 0)", rather than "aec_fg_rgb(255, 0, 0)".
		This is in an attempt of reducing overload for formatting by printf, but notice that aec_fg_rgb_l is more
		restricted than its standard version.
	*/


/*	Defines the methods really in case the flag is raised before
	inclusion. */
#ifdef ANSI_ESCAPE_CODE
// --------------------

// For printing: fputs and printf.
#include <stdio.h>		


/*	A wrapper around printing string in <ansi-esc.h>.

	Currently it is a "fputs", for, "printf" is unnecessary and
	"puts" prints '\n' at the end of the character sequences, while
	being uncalled for. */
#define _print_sequence(s)	fputs(s, stdout)

// AEC control sequence introducer.
#define CSI					"\e["

// AEC control sequence concluder.
#define CSC					"m"


// AEC sequence for resetting all attributes.
#define _AEC_RESET			CSI "0" CSC	

// Resets all the attributes on the text output.
#define aec_reset()			_print_sequence(_AEC_RESET)


// Cursor and terminal screen sequences
// ====================================

// Virtual Terminal
// ----------------

#define _AEC_INSERT			CSI "2"			// "Insert" VT key.
#define _AEC_DELETE			CSI "3"			// "Delete" VT key.
#define _AEC_END			CSI "4"			// "End" VT key.
#define _AEC_HOME			CSI "1"			// "Home" VT key.

// "Moves the current screen to terminal's buffer."
#define _AEC_BUFFER_SCREEN	_AEC_INSERT "J"

// "Erases terminal's buffered screen."
#define _AEC_ERASE_BUFFER	_AEC_DELETE "J"

// "Move to line 1 column 1".
#define _AEC_BEGINNING		"\e[H"			


#define _AEC_ERASE_CSR_END	CSI "0J"
#define _AEC_ERASE_CSR_BEG	CSI "1J"

#define _AEC_ERASE_CSR_END_LINE			CSI "0K"
#define _AEC_ERASE_BEG_CSRLINE_LINE		CSI	"1K"
#define _AEC_ERASE_LINE_LINE			CSI	"2K"

#define _AEC_CSR_LINE_BEG					'\r'

// AEC Sequence for cleaning the screen.
#define __CLEAN_SCREEN	_AEC_BEGINNING _AEC_BUFFER_SCREEN _AEC_ERASE_BUFFER

// Sequence for cleaning the (terminal) screen.
#define aec_clean()		printf(__CLEAN_SCREEN)


// Blinking state
// --------------

#define _AEC_SLOW_BLINK		CSI "5" CSC	
#define _AEC_RAPID_BLINK	CSI "6" CSC
#define _AEC_BLINK_OFF		CSI	"25" CSC

// TODO
#define aec_slow_blink()
#define aec_rapid_blink()
#define aec_blink_off()


// Cursor positioning
// ------------------

// Moves the cursor to the beggining of the terminal screen (line 1, column 1)>
#define aec_beginning()		_print_sequence(_AEC_BEGINNING)

// Moves the cursor to the beggining of it line on screen (to column 1).
#define aec_linebeg()		putc(CSR_LINE_BEG)

#define _AEC_CRS_UP(n)		CSI #n "A"
#define _AEC_CRS_DOWN(n)	CSI #n "B"
#define _AEC_CRS_RIGHT(n)	CSI #n "C"
#define _AEC_CRS_LEFT(n)	CSI #n "D"

/*	The position adjustment of the cursor by literal values takes
in consideration that it can be done directly without previous formatting.
For that, it should perhaps be more efficient, when that is the case. */

// Moves the cursor up by n positions. 
// The parameter must be literal.
#define aec_crs_up_l(n)		_print_sequence( _AEC_CRS_UP(n)    )	

// Moves the cursor down by n positions. * n must be literal.
#define aec_crs_down_l(n)	_print_sequence( _AEC_CRS_DOWN(n)  )	

// Moves the cursor right by n positiions. * n must be literal.
#define aec_crs_right_l(n)	_print_sequence( _AEC_CRS_RIGHT(n) )	

// Moves the cursor left by n positions. * n must be literal.
#define aec_crs_left_l(n)	_print_sequence( _AEC_CRS_LEFT(n)  )	

#define aec_crs_up(n)		printf(	CSI "%dA", (int) n )	// Moves the cursor up by n positions.
#define aec_crs_down(n)		printf(	CSI "%dB", (int) n )	// Moves the cursor down by n positions.
#define aec_crs_right(n)	printf(	CSI "%dC", (int) n )	// Moves the cursor right by n positions.
#define aec_crs_left(n)		printf(	CSI "%dD", (int) n )	// Moves the cursor left by n positions.


// Cursor tracking
// ---------------

#define aec_crs_request()	_print_sequence(CSI "6n")	// Returns as "<CSI>#;#R".

// Saves the current position of the cursor into a buffer.
#define aec_crs_save()		_print_sequence(CSI "s")

// Loads a position from a previously saved on the buffer and moves the cursor to it.
#define aec_crs_load()		_print_sequence(CSI "u")


// Graphical sequences
// ===================

/*	Default foreground and background coloring: The colors that are embedded in
	most terminal systems. */

#define _AEC_FG_BLACK	CSI "0;30" CSC
#define _AEC_FG_RED		CSI "0;31" CSC
#define _AEC_FG_GREEN	CSI "0;32" CSC
#define _AEC_FG_YELLOW	CSI "0;33" CSC
#define _AEC_FG_BLUE	CSI "0;34" CSC
#define _AEC_FG_PURPLE	CSI "0;35" CSC
#define _AEC_FG_CYAN	CSI	"0;36" CSC
#define _AEC_FG_WHITE	CSI "0;37" CSC

#define aec_fg_white()	_print_sequence(_AEC_FG_WHITE)
#define aec_fg_black()	_print_sequence(_AEC_FG_BLACK)
#define aec_fg_yellow()	_print_sequence(_AEC_FG_YELLOW)
#define aec_fg_red()	_print_sequence(_AEC_FG_RED)
#define aec_fg_blue()	_print_sequence(_AEC_FG_BLUE)
#define aec_fg_green()	_print_sequence(_AEC_FG_GREEN)
#define aec_fg_purple()	_print_sequence(_AEC_FG_PURPLE)
#define aec_fg_cyan()	_print_sequence(_AEC_FG_CYAN)

// TODO: background


/*	A more custom foreground and background coloring. Colors by specifing
the values in RGB. */

#define _AEC_FG_PREFFIX			CSI "38;"
#define _AEC_BG_PREFFIX			CSI "48;"

#define _AEC_RGB_L_CONCLUDER(r, g, b)		"2;" #r ";" #g ";" #b CSC

// Changes the foreground color attribute to a specified RGB color.
// The values must be literal unsigned byte integral constant expressions.
#define aec_fg_rgb_l(r, g, b)	_print_sequence(	_AEC_FG_PREFFIX _AEC_RGB_L_CONCLUDER(r, g, b)	)

// Changes the background color attribute to a specified RGB color.
// The values must be literal unsigned byte integral constant expressions.
#define aec_bg_rgb_l(r, g, b)	_print_sequence(	_AEC_BG_PREFFIX _AEC_RGB_L_CONCLUDER(r, g, b)	)

// Changes the foreground color attribute to a specified RGB color.
#define aec_fg_rgb(r, g, b)		printf( _AEC_FG_PREFFIX "2;%hhu;%hhu;%hhu" CSC, (unsigned char) r, (unsigned char) g, (unsigned char) b )

// Changes the background color attribute to a specified RGB color.
#define aec_bg_rgb(r, g, b)		printf( _AEC_BG_PREFFIX "2;%hhu;%hhu;%hhu" CSC, (unsigned char) r, (unsigned char) g, (unsigned char) b )


// Text styles
// -----------

#define _AEC_SET_ITALIC		CSI "3" CSC
#define _AEC_RESET_ITALIC	CSI "23" CSC

#define aec_set_italic()	_print_sequence( _AEC_SET_ITALIC )
#define aec_reset_italic()	_print_sequence( _AEC_RESET_ITALIC )


#define aec_bold()		// * NYI.
#define aec_unbold()	// * NYI.



#else	// from ANSI_ESCAPE_CODE. 

		// Definitions boilerplate
		// =======================


#define aec_reset()
#define aec_clean()

#define aec_beginning()
#define aec_linebeg()	

#define aec_crs_up(n)	
#define aec_crs_down(n)		
#define aec_crs_right(n)	
#define aec_crs_left(n)		
#define aec_crs_up_l(n)	
#define aec_crs_down_l(n)		
#define aec_crs_right_l(n)	
#define aec_crs_left_l(n)

#define aec_crs_request()
#define aec_crs_save()		
#define aec_crs_load()	

#define aec_fg_white()	
#define aec_fg_black()	
#define aec_fg_yellow()	
#define aec_fg_red()	
#define aec_fg_blue()	
#define aec_fg_green()	
#define aec_fg_purple()	
#define aec_fg_cyan()
#define aec_bg_white()	
#define aec_bg_black()	
#define aec_bg_yellow()	
#define aec_bg_red()	
#define aec_bg_blue()	
#define aec_bg_green()	
#define aec_bg_purple()	
#define aec_bg_cyan()

#define aec_fg_rgb(r, g, b)
#define aec_fg_rgb_l(r, g, b)
#define aec_bg_rgb(r, g, b)
#define aec_bg_rgb_l(r, g, b)

#define aec_set_italic()
#define aec_reset_italic()


#endif	// ANSI_ESCAPE_CODE
#endif	// _ANSI_ESCAPE_CODE_HEADER_
