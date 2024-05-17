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



// Registry and Data
// -----------------

// Registry's data_2 char-array size.
#define RD2_SIZE	1000

// Registry's data_3 char-array size.
#define RD3_SIZE	5000

// The registry key type. An integer <int>.
typedef int32_t key_t;


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


/*	Stores the result of searching processes. */
typedef struct {
	// The return registry in the search.
    registry_t target;
	// ^ 6016 [bytes], aligning by 8.

	/*	Holds on temporal measures about the application of searching. */
    struct {
		/*	The time it takes to construct the data-structures in order
			to do the search. */
        double construction_time;

		/*	The time the search itself takes to completion. */
        double time_span;
    } measures;
} search_result;	// Occupies 6032 [bytes]. Allign by 8 [bytes].


/*	A structure representing a pointer a to a registry in the registries data-stream.
	Holds on the registry key information and its pointer sequentially onto the file. */
typedef struct {
	uint32_t original_pos; 	// The pointer to the position of the registry in the registries-file.
	key_t key;				// The registry key itself.
} registry_pointer;



// Files and strreams
// ------------------

/*	A stream representing the registries file. */
typedef FILE	REG_STREAM;

/*	A stream representing the b-tree data-structure. */
typedef FILE	B_STREAM;

/*	A stream representing the b*-tree data-structure. */
typedef FILE	BSTAR_STREAM;


// The default registries filename.
#define INPUT_DATAFILENAME		    "input-data.bin"

// The default b-tree data-structure filename.
#define OUTPUT_BTREE_FILENAME	    "data.btree"

// The default b*-tree data-structure filename.
#define OUTPUT_BSTAR_FILENAME       "data.bstar"

// The default binary tree data-structure filename.
#define OUTPUT_EBST_FILENAME        "data.ebst"

// TODO: (Ponder)
#define OUTPUT_ISS_FILENAME         "data.iss"



// The key of the page is simply the key of first one by its itens.
#define page_key(p)			(p.itens[0].key)	





#endif // _COMMONS_HEADER_
