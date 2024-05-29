/*	<src/commons.h>
	
	Where common, universal definitions are given, and dependencies imported. */


#ifndef _ES_COMMONS_HEADER_
#define _ES_COMMONS_HEADER_


/*	The boolean type is globally used throughout programs. */
#include <stdbool.h>

/*	For size-specified integer types. */
#include <inttypes.h>

// TODO: Move.
#include "log.h"


// Conveniences
// ------------

// Modifier for forcing function inlining.
// Let "finline void func(...) { ... }" on its definition.
// * Definition compatible with GNU gcc compiler.
#define finline					__attribute__((always_inline)) inline

/* 	Returns whether a number x in between two numbers: a and b.
	Note that b >= a in order to a valid verification be done;
	otherwise the result will always be false. */
#define in_range(a, b, x)       (((b) >= x) && (x >= (a)))

/*	Returns the ceil of the division between x and y,
	where x and y are either of integer or natural type.
	ceil{x / y}. */
#define ceil_div(x, y)			((x / y) + ((x % y) ? 1 : 0))

/*  Modular increment. */
#define mod_incr(x, m)			((x + 1) % m)








#define TRANSPARENT_COUNTER	true


#ifdef TRANSPARENT_COUNTER
extern struct __transparent_counter_t {
	struct {
		uint64_t read;
		uint64_t write;
	} reg;

	struct {
		uint64_t read;
		uint64_t write;
	} ebst;

	struct {
		uint64_t read;
		uint64_t write;
	} erbt;

	struct {
		uint64_t read;
		uint64_t write;
	} b;

	struct {
		uint64_t read;
		uint64_t write;
	} bs;

	struct {
		uint64_t search;
		uint64_t build;
	} comparisons;

} transparent_counter;


#define cmp_eq_build(key1, key2)	((key1 == key2)	&& (++ transparent_counter.comparisons.build))
#define cmp_bg_build(key1, key2)	((key1 > key2) && (++ transparent_counter.comparisons.build))
#define cmp_ls_build(key1, key2)	((key1 < key2) && (++ transparent_counter.comparisons.build))

#define cmp_eq_search(key1, key2) ((key1 == key2) && (++ transparent_counter.comparisons.search))
#define cmp_bg_search(key1, key2) ((key1 > key2) && (++ transparent_counter.comparisons.search))
#define cmp_ls_search(key1, key2) ((key1 < key2) && (++ transparent_counter.comparisons.search))

#else

#define cmp_eq_build(key1, key2)	(key1 == key2)
#define cmp_bg_build(key1, key2)	(key1 > key2)
#define cmp_ls_build(key1, key2)	(key1 < key2)

#define cmp_eq_search(key1, key2)	(key1 == key2)
#define cmp_bg_search(key1, key2)	(key1 > key2)
#define cmp_ls_search(key1, key2)	(key1 < key2)
#endif


#endif // _ES_COMMONS_HEADER_
