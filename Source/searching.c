/* <searching.c>
	
	...
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "searching.h"


#define DATA_FILEPATH	"../Data/nothing.bin"


// Attempts opening a file in binary-read mode. Returns success.
inline static bool openFileReadBin(const char * __Filepath, FILE ** __ReturnFile) {
	* __ReturnFile = fopen(__Filepath, "rb");
	if (* __ReturnFile == NULL)
		return false;
	return true;
}


static inline bool assembleNumeral(const char * __NumberString, int * __Return) {
	short sign = 1;

	size_t iterator = 0, accumulator = 0;

	if (__NumberString[iterator] == '-') 
	{	// Negative sign.

		sign = -1;
		iterator ++;
	}
	else if (__NumberString[iterator] == '+') 
	{	// Positive sign.

		iterator ++;
	}

	while (__NumberString[iterator]) {
		if (is_numeral(__NumberString[iterator])) {
			accumulator *= NUMERICAL_BASE;
			// accumulator += __NumberString[iterator] - 48;
			accumulator += fromCharDigit(__NumberString[iterator]); // * for legibility?

			iterator ++;
			continue;
		}
		return false;
	}
	* __Return = (int) sign * accumulator;
	return true;
}


/*
	* SPECS.
*/
int main(int argc, char ** argsv)
{

	// Args

	int search_key = 0;
	if (argc > 1) {
		if (! assembleNumeral(argsv[1], & search_key)) {
			search_key = 0;
		}
	}

	// Initialization.
	
	FILE * data_file = NULL;
	if (! openFileReadBin(DATA_FILEPATH, & data_file)) {
		fprintf(stderr, "Error: Couldn't open data-file.\n");
		return EXIT_FAILURE;
	}

	// Pre-processing.

	PagesIndexTable table = assemblePIT(data_file);

	// Main Instructions.

	registry_t possible_item;
	search_response sr = indexed_seq_search(search_key, & table, data_file, & possible_item);

	if (sr == SEARCH_SUCCESS) {
		printf("Search Sucess!\n");
	} else {
		// TODO: DEBUG

		printf("Search failure. Code: <%d>.\n", (int) sr);
	}

	// Finalization.

	DestroyPIT(& table);
	fclose(data_file);

	printf("\nProgram finished ~~\n");

	return EXIT_SUCCESS;
}