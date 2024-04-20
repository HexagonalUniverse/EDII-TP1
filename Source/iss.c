/* <iss.c>

	Indexed sequential search.
*/

#include <stdio.h>
#include <stdlib.h>
#include "searching.h"


#define _ISS_BIN_SEARCH	true


static bool search_item_in_page(const int key, const page_t * page, registry_t * __ReturnItem) 
{
#if ! _ISS_BIN_SEARCH	// Sequential search.
	
	for (size_t i = 0; i < itens_per_page; i ++)
	{

		// Invariant: null-reg marks end of page valid entries.
		if (page -> itens[i].key == null_reg.key)
			break;

		if (page -> itens[i].key == key) {
			* __ReturnItem = page -> itens[i];
			return true;
		}
		else if (page -> itens[i].key > key)
			return false;
	}
	return false;
#else					// Binary search.
	

	size_t length = 0;
	while (length < itens_per_page && page -> itens[length].key != null_reg.key)
		length ++;
		
#define next_bs_middle_pos(x, y)	(x + ((y - x) >> 1))

	for (size_t l = 0, r = length - 1, m = length >> 1; r >= l; m = next_bs_middle_pos(l, r))
	{	
		if (page -> itens[m].key == key) {
			* __ReturnItem = page -> itens[m];
			return true;
		}

		else if (key > page -> itens[m].key)
			l = m + 1;

		else 
			r = m - 1;
	}
	return false;
#endif
}


/*
	...
*/
search_response indexed_seq_search
(
	const int key, const PagesIndexTable * __Table, FILE *  __DataFile,
	registry_t * __ReturnItem
) {
	/*
	TODO: (Documentation) Invariants and logic.
	*/

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
	if (! read_page(__DataFile, iterator - 1, & target_page))
		return PAGE_SEARCH_FAIL;

	// 4. Search for the item in the page.

	if (search_item_in_page(key, & target_page, __ReturnItem))
		return SEARCH_SUCCESS;

	// 5. If none of the cases above fell down, then the search were not successful.
	return SEARCH_END;
}