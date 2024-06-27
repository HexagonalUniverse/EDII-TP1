/*	<src/registry.h>

	Define the registry data-type. */


#ifndef _ES_REGISTRY_HEADER_
#define _ES_REGISTRY_HEADER_


#include "commons.h"

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

	key_t key;

	struct {	// The data.
		int64_t data_1;
		char data_2[RD2_SIZE];
		char data_3[RD3_SIZE];
	};
} registry_t;		// Occupies 6016 [bytes]. Allign by 8 [bytes].


/*	A structure representing a pointer a to a registry in the registries data-stream.
	Holds on the registry key information and its pointer sequentially onto the file. */
typedef struct {
	uint32_t original_pos; 	// The pointer to the position of the registry in the registries-file.
	key_t key;				// The registry key itself.
} registry_pointer;


#endif // _ES_REGISTRY_HEADER_
