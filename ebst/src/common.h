#ifndef _COMMON_DEF_HEADER_
#define _COMMON_DEF_HEADER_


#include <inttypes.h>


#define finline         __attribute__((always_inline)) inline


#define ORDFILENAME     "ord-data.bin"
#define UNORDFILENAME   "unord-data.bin"

typedef const char * filepath;


// Registry def.
// -------------

#define RD2_SIZE	    1000    // Registry data_2 array size.
#define RD3_SIZE	    5000    // Registry data_3 array size.


// A representation of a single registry in the database.
typedef struct {
	int key;		// The information for localization.

	int64_t data_1; char data_2[RD2_SIZE]; char data_3[RD3_SIZE];
} registry_t;       // Occupies 6016 [bytes]. Allign by 8 [bytes].


#define INT_MAX				((int) (1 << 31) - 1)
#define midpoint(l, r)		(l + ((r - l) >> 1))


#define in_range(a, b, x)   ((b >= x) && (x >= a))


typedef struct {
    registry_t root_item;

    int left, right;
} ebst_node;


#endif // _COMMON_DEF_HEADER_
