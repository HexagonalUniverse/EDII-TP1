/*	external-search.cpp
	
	TODO: To describe.


	* Currently blank source file.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "logging.h"


// Register and Data
// -----------------

// Register Data 2 array size.
#define RD2_SIZE	1000

// Register Data 3 array size.
#define RD3_SIZE	5000


// (...)
typedef struct {
	int key;		// The key for searching.

	long data_1;	
	char data_2[RD2_SIZE];
	char data_3[RD3_SIZE];
} register_t;	// 6008 [bytes]




// Pagination System
// -----------------

// How many itens, at its maximum, will hold each page.
#define itens_per_page		4

// How many frames are there to be stored in the pagination-system.
#define page_frame_size		10


typedef struct {
	// "Number of itens": How many itens are within the page.
	size_t n_itens;

	register_t itens[itens_per_page];
} page_t;


// The key of the page is simply the key of first one by its itens.
#define page_key(p)	(p.itens[0].key)

// Retrieves the last item (registry) in the page.
#define page_last(p) (p.itens[p.n_itens - 1].key)


typedef struct {
	page_t frames[page_frame_size];
	size_t size;
} PageStack;




bool indexed_seq_search(const int key, const PageStack * stack, page_t * return_page)
{

	// 1. Indexed Sequential Search over the page's frames-stack.
	// (...)
	size_t iterator = 0;
	while (iterator < stack -> size && page_key(stack -> frames[iterator]) <= key)
		iterator ++;

	// 2. Page validation step.
	// If the item is not in any possible frame in the stack, then it is not there.
	// 1
	if (iterator == 0) {
		ISS_1_1();
		return false;
	}

	else if (key > page_last(stack -> frames[iterator - 1])) {
		ISS_1_2();
		return false;
	}

	page_t page = stack -> frames[iterator - 1];

	printf("\t> Perhaps the item on searching is on the following page:\n\t>\t");
	// PrintPage(page);
	printf("\n\n");
		
	// 3. Sequential search on the found page: is the key in there?
	for (size_t i = 0; i < page.n_itens; i++)
		if (page.itens[i].key == key) 
		{
			ISS_3();

			* return_page = page;
			return true;
		}


	// 4. TODO: To complete.

	ISS_4();

	return false;
}








int main(int argc, char ** argsv) 
{





	return EXIT_SUCCESS;
}

//