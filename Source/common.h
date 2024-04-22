/* <common.h>
	
	Where common definitions used over the program are done.
*/


#ifndef _COMMON_DEFS_HEADER_
#define _COMMON_DEFS_HEADER_


#include <stdbool.h>
#include <inttypes.h>


#ifndef INT_MAX
	#define INT_MAX		((int) ((unsigned) - 1))
#endif


// Modifier for forcing function inlining.
// * Definition compatible with gcc.
#define finline			__attribute__((always_inline)) inline



// Registry and Data
// -----------------

// Registry data_2 array size.
#define RD2_SIZE	1000

// Registry data_3 array size.
#define RD3_SIZE	5000


// A representation of a single registry in the database.
typedef struct {
	int key;		// The information for localization.

	int64_t data_1;
	char data_2[RD2_SIZE];
	char data_3[RD3_SIZE];
} registry_t;		// Occupies 6016 [bytes]. Allign by 8 [bytes].


#define NULLREG_KEY		INT_MAX;

// The identitary null registry.
extern const registry_t null_reg;



// String processing
// -----------------


typedef const char * filepath;

#define in_range(a, b, x)	((b >= x) && (x >= a))
#define nin_range(a, b, x)	(! in_range(a, b, x))

#define is_numeral(x)		in_range(48, 57, x)

#define NUMERICAL_BASE		10

#define fromCharDigit(x)	(x - 48)


#endif // _COMMON_DEFS_HEADER_
