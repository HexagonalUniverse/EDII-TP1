/*	<src/include/commons.h>
	
	Where common, universal definitions are given, and dependencies imported,
	for the external-search project. */


#ifndef _ES_COMMONS_HEADER_
#define _ES_COMMONS_HEADER_


#include <stdbool.h>	/*	The boolean type is globally used. */
#include <inttypes.h>	/*	For size-specified integer types. */

/*	Every further part of the system import consequentially the 
	logging system and the occasional transparent counter.*/
#include "log.h"
#include "transparent-counter.h"


// Conveniences
// ------------

// Definition compatible with GNU gcc compiler.
// Modifier for forcing function inlining.
// Let "finline void func(...) { ... }" on its definition.
#define finline					__attribute__((always_inline)) inline

/* 	Returns whether a number x in between a and b.
	Note that b >= a in order to a valid verification to be done.
	Unsafe in side-effects for x. */
#define in_range(a, b, x)		(((b) >= (x)) && ((x) >= (a)))

/*	Returns whether a number x is NOT between a and b.
	Note that b >= a in order to a valid verification to be done.
	Unsafe in side-effects for x. */
#define nin_range(a, b, x)		(((b) < (x)) || ((x) < (a)))

/*	Returns the ceil of the division between x and y,
	where x and y are either of integer or natural type.
	ceil{x / y}. */
#define ceil_div(x, y)			((x / y) + ((x % y) ? 1 : 0))

/*  Modular increment. */
#define mod_incr(x, m)			((x + 1) % m)



#endif // _ES_COMMONS_HEADER_
