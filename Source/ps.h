/* <ps.h> 
	
	Pagination system header.
*/


#ifndef _PAGINATION_SYSTEM_HEADER_
#define _PAGINATION_SYSTEM_HEADER_


#include "common.h"
#include <stdio.h>


// How many itens, at its maximum, holds each page.
// TODO: Decide it more broadly.
#define itens_per_page		5

// How many page-frames are there to be stored in the pagination-system.
#define page_frame_size		10


// Represents a single page in the pagination-system.
// Structurally: an static array.
typedef struct {
	registry_t itens[itens_per_page];
} page_t; // Occupies 30080 [bytes]. Allign by 8 [bytes].


// The key of the page is simply the key of first one by its itens.
#define page_key(p)			(p.itens[0].key)

// Retrieves the last registry key in the page. * Changed
// #define page_last_key(p)	(p.itens[p.n_itens - 1].key)


// From a data file-stream, reads a single registry page, specified by 
// its index. Returns success.
bool readPage(FILE *, size_t index, page_t * __return);

// Loads a page into main memory by the page-frames.
// TODO: def
bool loadPage(void);


// A representational for the pages index-table, used for the ISS;
// structurally an array.
typedef struct {
	int * keys;
	size_t length;
} PagesIndexTable;

// Calculates how many pages are there given the registry quantity in data file.
#define FileNPages(n_itens)	((n_itens / itens_per_page) + (n_itens % itens_per_page ? 1 : 0))


PagesIndexTable assemblePIT(FILE *);	// * ds constructor
void DestroyPIT(PagesIndexTable*);		// * ds deconstructor



// Other tools
// -----------

// TODO: def
void printRegistry(void);

// TODO: def
void printPage(void);

// TODO: def
void printPagesIndexTable(void);



#endif // _PAGINATION_SYSTEM_HEADER_