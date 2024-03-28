/*	<external-search.c>
	
	TODO: (Documentation)	To describe.
	TODO: (Nomenclature)	Registry / Item. Are they synnonimous over program?

	* Currently not-functional source file. Incomplete.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>


// * Implementations defs

// TODO: (Nomenclature)		"PageStack" may not be the most appropriate name.
#define PAGESTACK_IMPLEMENTATION		false

#define PAGEINDEXTABLE_IMPLEMENTATION	false
#define PS_IO_IMPLEMENTATION			true



// Registry and Data
// -----------------

// Registry data_2 array size.
#define RD2_SIZE	1000

// Registry data_3 array size.
#define RD3_SIZE	5000


// A representation of a single registry in the database.
typedef struct {
	// TODO: (Refactoring)		int key -> key_t key	

	int key;		// The information for localization.

	long data_1;	
	char data_2[RD2_SIZE];
	char data_3[RD3_SIZE];
} registry_t; // Occupies 6008 [bytes]. Allign by 4 [bytes].


// Data-file header.
typedef struct {
	// * For instance, just contains information about its size.
	// It is, "how much itens are in there."

	int size;
} DF_header;



// Pagination-system [PS]
// ----------------------

// How many itens, at its maximum, holds each page.
#define itens_per_page		4

// How many frames are there to be stored in the pagination-system.
#define page_frame_size		10


typedef struct {
	// "Number of itens": How many itens are stored within the page.
	// size_t n_itens;

	registry_t itens[itens_per_page];
} page_t; // Occupies 24032 [bytes]. Allign by 4 [bytes].


// The key of the page is simply the key of first one by its itens.
#define page_key(p)			(p.itens[0].key)

// Retrieves the last registry key in the page.
#define page_last_key(p)	(p.itens[p.n_itens - 1].key)



// Pagination-system IO
// --------------------

#if PS_IO_IMPLEMENTATION
// Retrieves a page from the file stream. Returns success.
static inline bool read_page(FILE * __Stream, size_t index, page_t * __return_page) 
{
	fseek(__Stream, sizeof(DF_header) + index * sizeof(page_t), SEEK_SET);
	fread(__return_page, sizeof(page_t), 1, __Stream);
	return (ferror(__Stream) == 0);
}


// Blank.
#define goto_beggining_of_databody(__Stream)	
#endif



#ifdef PAGEINDEXTABLE_IMPLEMENTATION
typedef struct {
	int * keys;
	size_t length;
} PagesIndexTable;


static void PrintPagesIndexTable(const PagesIndexTable * __Table) {
	if (__Table -> length == 0) {
		printf("<>\n");
		return;
	}

	const size_t lm1 = __Table -> length - 1;
	
	putchar('<');
	for (size_t i = 0; i < lm1; i ++)
		printf("%8d, ", __Table -> keys[i]);
	printf("%8d>\n 1", __Table -> keys[lm1]);

	for (size_t i = 0; i < __Table -> length; i++)
		printf("       ");
	printf("%d\n", __Table -> length);
}


static inline void PrintRegistry(const registry_t * __Registry) {
	if (__Registry->key == INT_MAX) {
		printf("{ NULL }"); return;
	}


	printf("{ k = %d, d1 = %ld, |d2| = %u, |d3| = %u }",
		__Registry -> key, __Registry -> data_1,
		strlen(__Registry -> data_2), strlen(__Registry -> data_3));
}


static void PrintPage(const page_t * __Page, const size_t id) {
	printf("Page %u:\n", id);
	for (size_t i = 0; i < itens_per_page; i++) {
		printf("\t| ");
		PrintRegistry(& __Page -> itens[i]);
		putchar('\n');
	}
}




static void DestroyPIT(PagesIndexTable * __Table) {
	if (__Table -> keys != NULL)
		free(__Table -> keys);
}




const registry_t null_reg = { INT_MAX, 0 };


static inline PagesIndexTable assemble_pages_index_table(FILE * __Stream) 
{
	PagesIndexTable table = { NULL, 0 };


	// Reading the data header.

	DF_header header;
	fread(& header, sizeof(DF_header), 1, __Stream);

	if (ferror(__Stream)) {
		printf("Error: [assemble_pages_index_table] header\n");
		return table;
	}
	
	#define NumberOfPages(n_itens)	((n_itens / itens_per_page) + (n_itens % itens_per_page ? 1 : 0))
	const size_t n_pages = NumberOfPages(header.size);

	// File data section.

	page_t dummy; size_t page_id = 1;
	
	while (page_id < n_pages) {
		fread(& dummy, sizeof(page_t), 1, __Stream);
		
		if (ferror(__Stream)) {
			printf("Error: reading page\n");
			break;
		}

		table.keys = (int *) realloc(table.keys, sizeof(int) * (++ table.length));
		table.keys[table.length - 1] = dummy.itens[0].key;
		
		
		printf(">\tread page\n");
		PrintPage(&dummy, page_id);

		page_id ++;
	}

	// The last page is a case apart, for which it can be incomplete.
	
	size_t i = 0;
	while (! feof(__Stream) && i < itens_per_page) {
		fread(& dummy.itens[i ++], sizeof(registry_t), 1, __Stream);

		if (ferror(__Stream)) {
			printf("Error: reading last page\n");
			break;
		}
		printf(">\tread item\n");
	}
	i--;

	// Fulfilling the last page with <NULL>.
	while (i < itens_per_page) dummy.itens[i ++] = null_reg;

	table.keys = (int*)realloc(table.keys, sizeof(int) * (++table.length));
	table.keys[table.length - 1] = dummy.itens[0].key;

	printf(">\tread page\n");
	PrintPage(&dummy, page_id);


	PrintPagesIndexTable(& table);

	return table;
}


typedef enum {
	SEARCH_SUCCESS,

	KEY_UNDERPASSED,
	KEY_OVERPASSED,

	PAGE_SEARCH_FAIL,

	SEARCH_END

} search_response;


/*
	...
*/
search_response indexed_seq_search
(
	const int key, const PagesIndexTable * __Table, FILE * __DataFile,
	registry_t * __ReturnItem
) {
	/* 
	TODO: (Documentation) Invariants and logic.
	*/

	printf("[indexed_seq_search]\n");

	* __ReturnItem = null_reg;

	// 1. Indexed sequential search over table.

	size_t iterator = 0;
	while (iterator < __Table -> length && __Table -> keys[iterator] <= key)
		iterator ++;

	// 2. Found page index validation.

	if (iterator == 0)
		return KEY_UNDERPASSED;

	// TODO: (Implementation)
	// * for when it is bigger than any other page.
	else if (false)
		return KEY_OVERPASSED;

	printf("iterator: %u\n", iterator);

	// 3. Retrieving the page found.
	// TODO: (Implementation) Including the pagination-system.
	
	page_t target_page;
	if (! read_page(__DataFile, iterator - 1, &target_page))
		return PAGE_SEARCH_FAIL;

	// 4. Search for the item in the page.
	// TODO: (Refactoring) Extract method.

	// * for instance: sequential search...
	for (size_t i = 0; i < itens_per_page; i ++) 
	{
		if (target_page.itens[i].key == null_reg.key)
			break;
		
		if (target_page.itens[i].key == key) {
			* __ReturnItem = target_page.itens[i];
			return SEARCH_SUCCESS;
		}
	}


	// 5. If none of the cases above fell down, then the search were not successful.
	return SEARCH_END;
}
#endif



int main(int argc, char ** argsv) 
{
	// Initialization.
	
	const char filepath[] = "../Data/nothing.bin";
	FILE * data_file = fopen(filepath, "rb");
	if (data_file == NULL) {
		printf("ERROR: Couldn't open file\n");
		return EXIT_FAILURE;
	}

	// Preprocessing.

	PagesIndexTable table = assemble_pages_index_table(data_file);

	// Main instructions.

	registry_t possible_item;
	search_response s = indexed_seq_search(1326051, &table, data_file, & possible_item);
	if (s == SEARCH_SUCCESS) {
		printf("SEARCH_SUCCESS!\n");
	}
	else {
		printf("Search failure. Code: <%d>.\n", s);
	}

	// Termination.

	DestroyPIT(& table);
	fclose(data_file);

	printf("\nProgram finished ~~\n");
	
	return EXIT_SUCCESS;
}


// TODO: (Documentation)	signing and stuff...