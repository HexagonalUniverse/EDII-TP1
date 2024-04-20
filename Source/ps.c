/* <ps.c>

	Pagination system source.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ps.h"


// Retrieves a single page from the file stream. Returns success.
inline bool read_page(FILE * __Stream, size_t index, page_t * __return_page)
{
	fseek(__Stream, sizeof(DF_header) + index * sizeof(page_t), SEEK_SET);
	fread(__return_page, sizeof(page_t), 1, __Stream);
	return (ferror(__Stream) == 0);
}


void DestroyPIT(PagesIndexTable * __Table) {
	if (__Table -> keys != NULL)
		free(__Table -> keys);
}


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
	if (__Registry -> key == INT_MAX) {
		printf("{ NULL }"); return;
	}


	printf("{ k = %d, d1 = %ld, |d2| = %u, |d3| = %u }",
		__Registry -> key, (long)	 __Registry -> data_1,
		strlen(__Registry -> data_2), strlen(__Registry -> data_3));
}


static void PrintPage(const page_t * __Page, const size_t id) {
	printf("Page %u:\n", id);
	for (size_t i = 0; i < itens_per_page; i ++) {
		printf("\t| ");
		PrintRegistry(& __Page -> itens[i]);
		putchar('\n');
	}
}



const registry_t null_reg = { INT_MAX, 0 };


inline PagesIndexTable assemblePIT(FILE * __Stream)
{
	PagesIndexTable table = { NULL, 0 };


	// Reading the data header.

	fseek(__Stream, 0, SEEK_SET);

	DF_header header;
	fread(& header, sizeof(DF_header), 1, __Stream);

	if (ferror(__Stream)) {
		fprintf(stderr, "E: [assemblePIT]\tHeader.\n");
		return table;
	}

	const size_t n_pages = NumberOfPages(header.size);

	// File data section.

	page_t dummy; size_t page_id = 1;

	while (page_id < n_pages) {
		fread(& dummy, sizeof(page_t), 1, __Stream);

		if (ferror(__Stream)) {
			fprintf(stderr, "E [assemblePIT]\tReading the page inner loop.\n");
			break;
		}

		table.keys = (int *) realloc(table.keys, sizeof(int) * (++ table.length));
		table.keys[table.length - 1] = dummy.itens[0].key;

		printf(">\tread page\n");
		PrintPage(& dummy, page_id);

		page_id ++;
	}

	// The last page is a case apart, for which it can be incomplete.

	size_t i = 0;
	while (! feof(__Stream) && i < itens_per_page) {
		fread(& dummy.itens[i ++], sizeof(registry_t), 1, __Stream);

		if (ferror(__Stream)) {
			fprintf(stderr, "E: [assemblePIT]\tReading the last page.\n");
			break;
		}
		printf(">\tread item\n");
	}
	i --;

	// Fulfilling the last page with <NULL>.
	while (i < itens_per_page) dummy.itens[i ++] = null_reg;

	table.keys = (int *)realloc(table.keys, sizeof(int) * (++ table.length));
	table.keys[table.length - 1] = dummy.itens[0].key;

	printf(">\tread page\n");
	PrintPage(&dummy, page_id);

	PrintPagesIndexTable(&table);

	return table;
}