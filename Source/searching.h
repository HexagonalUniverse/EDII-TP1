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


search_response indexed_seq_search(const int _key, const PagesIndexTable *, 
	FILE *, registry_t * __ReturnItem);


#endif // _SEARCHING_HEADER_