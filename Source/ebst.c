
#include "searching.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// #define circular_inclusive_loop(s, e, m)	for (size_t i = s; i <= e; i = (i + 1) % m)


static inline bool isPageInPF(const PageFrames * PF, const int __PageKey, int * __ReturnIndex) {
	for (size_t i = PF -> _start; i <= PF -> _end; i = (i + 1) % pages_on_frame {
		if (page_key(PF -> page_heap[i]) == __PageKey) {
			* __ReturnIndex = i;
			return true;
		}
	}
	return false;
}


static inline bool retrievePage(PageFrames * PF, FILE * __Stream, const int __PageKey, int * __ReturnIndex) {
	if (isPageInPF(PF, __PageKey, __ReturnIndex))
		return true;

	page_t target_page;
	if (! read_page(__Stream, __PageKey, & target_page))
		return false;


}



search_response ebst_search(const int _key, PageFrames * PF, FILE * __Stream, registry_t * __ReturnItem) 
{


}



