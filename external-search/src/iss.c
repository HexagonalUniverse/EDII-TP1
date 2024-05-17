
#include "iss.h"


/* 

    <TEMPLATE USANDO A TABELA DE PÁGINAS>

search_response indexed_seq_search
(
	const int key, const PagesIndexTable * __Table, FILE *  __DataFile,
	registry_t * __ReturnItem
) {
	* __ReturnItem = null_reg;

	// 1. Indexed sequential search over table.

	size_t iterator = 0;
	while (iterator < __Table -> length && __Table -> keys[iterator] <= key)
		iterator ++;

	// 2. Found page index validation.
	
	if (iterator == 0)	// Less than any other key;
		return KEY_UNDERPASSED;
	
	else if (false)		// More than any other key. 
		return KEY_OVERPASSED;

	// 3. Retrieving the page found.

	page_t target_page;
	if (! readPage(__DataFile, iterator - 1, & target_page))
		return PAGE_SEARCH_FAIL;

	// 4. Search for the item in the page.

	if (search_item_in_page(key, & target_page, __ReturnItem))
		return SEARCH_SUCCESS;

	// 5. If none of the cases above fell down, then the search were not successful.
	return SEARCH_EXHAUSTION;
}
*/


/*  */
bool indexedSequencialSearch_OrderedAscending(const key_t _Key, FILE * _Stream, search_result * _Sr)
{
    printf("%d%p%p\n", _Key, _Stream, _Sr);
    return false;
}


/*  */
bool indexedSequencialSearch_OrderedDescending(const key_t _Key, FILE * _Stream, search_result * _Sr)
{
    printf("%d%p%p\n", _Key, _Stream, _Sr);
    return false;
}

