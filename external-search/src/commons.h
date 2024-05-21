// <commons.h>

#ifndef _COMMONS_HEADER_
#define _COMMONS_HEADER_


#include <stdio.h>
#include <stdlib.h>		
#include <stdbool.h>
#include <inttypes.h>
#include "log.h"


// Conveniences
// ------------

// Modifier for forcing function inlining.
// Let "finline void func(...) { ... }" on its definition.
// * Definition compatible with GNU gcc compiler.
#define finline			__attribute__((always_inline)) inline

/* 	Returns whether a number x in between two numbers: a and b.
	Note that b >= a in order to a valid verification be done;
	otherwise the result will always be false. */
#define in_range(a, b, x)       (((b) >= x) && (x >= (a)))

/*	Returns the ceil of the division between x and y,
	where x and y are either of integer or natural type.
	ceil{x / y}. */
#define ceil_div(x, y)	((x / y) + ((x % y) ? 1 : 0))

/*  Modular increment. */
#define mod_incr(x, m)				((x + 1) % m)


// Registry and Data
// -----------------

// Registry's data_2 char-array size.
#define RD2_SIZE	1000

// Registry's data_3 char-array size.
#define RD3_SIZE	5000

// The registry key type. An integer <int>.
typedef int32_t		key_t;


/*	A representation of a single registry in the database. */
typedef struct {
	/*	For the field <data_1> aligning by 8 [bytes],
		the overall structure size is 4 [bytes] more than it need to be.
		In practice, only 6012 [bytes] are used. */

	key_t key;		// The information for localization.

	int64_t data_1;
	char data_2[RD2_SIZE];	
	char data_3[RD3_SIZE];
} registry_t;		// Occupies 6016 [bytes]. Allign by 8 [bytes].


/*	A structure representing a pointer a to a registry in the registries data-stream.
	Holds on the registry key information and its pointer sequentially onto the file. */
typedef struct {
	uint32_t original_pos; 	// The pointer to the position of the registry in the registries-file.
	key_t key;				// The registry key itself.
} registry_pointer;


// Searching
// ---------

/*	Stores the result of searching processes. */
typedef struct {
	// The return registry in the search.
    registry_t target;
	// ^ 6016 [bytes], aligning by 8.

	struct {
		bool success		: 1;
		uint32_t transference_count;
		uint32_t comparison_count;
	}; // flags

	/*	Holds on temporal measures about the application of searching. */
    struct {
		/*	The time it takes to construct the data-structures in order
			to do the search. */
        double construction_time;

		/*	The time the search itself takes to completion. */
        double time_span;
    } measures;
} search_result;	// Occupies 6040 [bytes]. Allign by 8 [bytes].


typedef enum {
	// Sinalizes the search went correctly and key holder could be found.
	SEARCH_SUCCESS,

	// Sinalizes the key holder couldn't be found in the search.
	SEARCH_FAILURE,

	/*	Spec. errors */

	// Specification contradiction in case an unordered file is presented to the ISS search-engine.
	_SE_UNORDERED_ISS,


	/*	File errors */

	_SE_REGDATAFILE,
	_SE_BFILE,
	_SE_BSTARFILE,
	_SE_EBST_FILE,

	/*	Data-structures errors */

	_SE_MAKEFRAME,
	_SE_INDEXTABLE,
	_SE_BBUILD,
	_SE_BSTARBUILD

} SEARCH_RESPONSE;


// Files and streams
// -----------------

/*	A stream representing the registries file. */
typedef FILE	REG_STREAM;

/*	A stream representing the b-tree data-structure. */
typedef FILE	B_STREAM;

/*	A stream representing the b*-tree data-structure. */
typedef FILE	BSTAR_STREAM;

/*	A stream representing the ebst data-structure. */
typedef FILE	EBST_STREAM;


// The default registries filename.
#define INPUT_DATAFILENAME			"temp/10.bin"
// #define INPUT_DATAFILENAME			"temp/input-data.bin"


// The default b-tree data-structure filename.
#define OUTPUT_BTREE_FILENAME	    "temp/data.btree"

// The default b*-tree data-structure filename.
#define OUTPUT_BSTAR_FILENAME       "temp/data.bstar"

// The default binary tree data-structure filename.
#define OUTPUT_EBST_FILENAME        "temp/data.ebst"

// TODO: (Ponder)
#define OUTPUT_ISS_FILENAME         "temp/data.iss"



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


#define cmp_eq_tc_build(key1, key2)	((key1 == key2) && (++ transparent_counter.comparisons.build))
#define cmp_bg_tc_build(key1, key2)	((key1 > key2) && (++ transparent_counter.comparisons.build))
#define cmp_ls_tc_build(key1, key2)	((key1 < key2) && (++ transparent_counter.comparisons.build))

#define cmp_eq_tc_search(key1, key2) ((key1 == key2) && (++ transparent_counter.comparisons.search))
#define cmp_bg_tc_search(key1, key2) ((key1 > key2) && (++ transparent_counter.comparisons.search))
#define cmp_ls_tc_search(key1, key2) ((key1 < key2) && (++ transparent_counter.comparisons.search))


#endif


#endif // _COMMONS_HEADER_
