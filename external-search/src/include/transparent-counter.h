/*	<src/include/transparent-counter.h>
	
	Toggles the transparent-counter for depurating the comparisons counting, 
	as well as the IO operation for the pages. */


#ifndef TRANSPARENT_COUNTER_H
#define TRANSPARENT_COUNTER_H


#include <stdbool.h>


#if ! defined(TRANSPARENT_COUNTER)
	// Toggles the transparent-counter.
	#define TRANSPARENT_COUNTER		false
#endif


#if TRANSPARENT_COUNTER
#include <stdint.h>


void transparent_counter_print(void);

struct page_transferences { uint64_t read, write; };

/*	A global data-structure for tracking the search-engines
	IO transferences and comparisons.

	Contains:
	-	number of pages read/written;
	-	number of comparasions while building and searching. */
extern struct __the_transparent_counter {
	struct page_transferences reg, ebst, erbt, b, bs;

	struct {
		uint64_t search;
		uint64_t build;
	} comparisons;

} transparent_counter;

/*	Encapsulating the comparisons into the transparent-counter DS. */

#define cmp_eq_build(key1, key2)	( (++ transparent_counter.comparisons.build) && (key1 == key2) )	// key1 == key2
#define cmp_bg_build(key1, key2)	( (++ transparent_counter.comparisons.build) && (key1 > key2) )		// key1 > key2
#define cmp_ls_build(key1, key2)	( (++ transparent_counter.comparisons.build) && (key1 < key2) )		// key1 < key2
#define cmp_be_build(key1, key2)	( (++ transparent_counter.comparisons.build) && (key1 >= key2) )	// key1 >= key2
#define cmp_le_build(key1, key2)	( (++ transparent_counter.comparisons.build) && (key1 <= key2) )	// key1 <= key2

#define cmp_eq_search(key1, key2)	( (++ transparent_counter.comparisons.search) && (key1 == key2) )	// key1 == key2	
#define cmp_bg_search(key1, key2)	( (++ transparent_counter.comparisons.search) && (key1 > key2) )	// key1 > key2
#define cmp_ls_search(key1, key2)	( (++ transparent_counter.comparisons.search) && (key1 < key2) )	// key1 < key2
#define cmp_be_search(key1, key2)	( (++ transparent_counter.comparisons.search) && (key1 >= key2) )	// key1 >= key2
#define cmp_le_search(key1, key2)	( (++ transparent_counter.comparisons.search) && (key1 <= key2) )	// key1 <= key2

#else

#define cmp_eq_build(key1, key2)	(key1 == key2)	// key1 == key2
#define cmp_bg_build(key1, key2)	(key1 > key2)	// key1 > key2
#define cmp_ls_build(key1, key2)	(key1 < key2)	// key1 < key2
#define cmp_be_build(key1, key2)	(key1 >= key2)	// key1 >= key2
#define cmp_le_build(key1, key2)	(key1 <= key2)	// key1 <= key2

#define cmp_eq_search(key1, key2)	(key1 == key2)	// key1 == key2
#define cmp_bg_search(key1, key2)	(key1 > key2)	// key1 > key2
#define cmp_ls_search(key1, key2)	(key1 < key2)	// key1 < key2
#define cmp_be_search(key1, key2)	(key1 >= key2)	// key1 >= key2
#define cmp_le_search(key1, key2)	(key1 <= key2)	// key1 <= key2
#endif

#endif