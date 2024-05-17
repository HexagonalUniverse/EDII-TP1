#ifndef _COMMON_HEADER_
#define _COMMON_HEADER_


#include <stdbool.h>
#include <inttypes.h>


// Registry and Pages
// ------------------

#define RD2_SIZE	1000
#define RD3_SIZE	5000


typedef int32_t key_t;


typedef struct {
	key_t key;

	int64_t data_1;
	char data_2[RD2_SIZE];
	char data_3[RD3_SIZE];
} registry_t;	// 6016 [bytes]


// A structure representing a pointer a to a registry in a data-stream.
// Holds on the registry key information and its pointer sequentially onto the file.
typedef struct {
	uint32_t original_pos; 	// Stores the pointer which holds the position of the registry in the original file.
	key_t key;				// The registry key itself.
} registry_pointer;


#define NULLREG_KEY		INT_MAX;
extern const registry_t null_reg;


// ---

#define ITENS_PER_PAGE		5					// How many itens, at its maximum, holds each page.
#define PAGE_FRAME_SIZE		10					// How many page-frames are there to be stored in the pagination-system.


// Represents a single page in the pagination-system. Structurally: an static array.
typedef struct {
	registry_t itens[ITENS_PER_PAGE];
} page_t; // Occupies 30080 [bytes]. Allign by 8 [bytes].

// The key of the page is simply the key of first one by its itens.
#define page_key(p)			(p.itens[0].key)	


// From a data file-stream, reads a single registry page, specified by 
// its index. Returns whether the page had been read in its entirety; with its [ITENS_PER_PAGE] itens.
size_t readPage(FILE * _Stream, size_t _Index, page_t * _ReturnPage);

// Loads a page into main memory on the page-frames. TO DEFINE!
// bool loadPage(void);



#endif // _COMMON_HEADER_
