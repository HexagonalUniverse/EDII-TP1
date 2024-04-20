/* <searching.h>
	
	...
*/


#ifndef _SEARCHING_HEADER_
#define _SEARCHING_HEADER_


#include "ps.h"


typedef enum {
	SEARCH_SUCCESS,

	KEY_UNDERPASSED,
	KEY_OVERPASSED,

	PAGE_SEARCH_FAIL,

	SEARCH_END

} search_response;


// ISS

search_response indexed_seq_search(const int _key, const PagesIndexTable *, 
	FILE *, registry_t * __ReturnItem);


// EBST

#define pages_on_frame	4


// * lru
typedef struct {
	page_t page_heap[pages_on_frame];
	size_t pages_index[pages_on_frame];

	size_t _start, _end;
} PageFrames;


#endif // _SEARCHING_HEADER_