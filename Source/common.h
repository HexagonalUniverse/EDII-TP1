/* <common.h>
	
	(...)
*/


#ifndef _COMMON_DEFS_HEADER_
#define _COMMON_DEFS_HEADER_


#include <stdbool.h>
#include <inttypes.h>


#define INT_MAX		((int) - 1)


// Registry and Data
// -----------------

// Registry data_2 array size.
#define RD2_SIZE	1000

// Registry data_3 array size.
#define RD3_SIZE	5000


// A representation of a single registry in the database.
typedef struct {
	// TODO: (Refactoring)		int key -> key_t key	

	int key;		// The information for localization.

	int64_t data_1;
	char data_2[RD2_SIZE];
	char data_3[RD3_SIZE];
} registry_t; // Occupies 6008 [bytes]. Allign by 4 [bytes].


// The identitary null registry.
extern const registry_t null_reg;


// Data-file header.
typedef struct {
	// * For instance, just contains information about its size.
	// It is, "how much itens are in there."

	int size;
} DF_header;



// String processing
// -----------------

#define in_range(a, b, x)	((b >= x) && (x >= a))
#define is_numeral(x)		in_range(48, 57, x)

#define NUMERICAL_BASE		10

#define fromCharDigit(x)	(x - 48)


#endif // _COMMON_DEFS_HEADER_
