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

	SEARCH_EXHAUSTION,
	
} search_response;



search_response indexed_seq_search(const int _key, const PagesIndexTable *, 
	FILE *, registry_t * __ReturnItem);

search_response ebst_search(const int _key, FILE * __ebstStream, registry_t * __Return);





#endif // _SEARCHING_HEADER_