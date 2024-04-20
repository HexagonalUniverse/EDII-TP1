/* <searching.c>
	
	...
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "searching.h"


#define DATA_FILEPATH	"../Data/nothing.bin"


// A wrapper around opening a file in binary-read mode. Returns success.
inline static bool openFileReadBin(const char * __Filepath, FILE ** __ReturnFile) {
	* __ReturnFile = fopen(__Filepath, "rb");
	if (* __ReturnFile == NULL)
		return false;
	return true;
}


static inline bool assembleNumeral(const char * __NumberString, int * __Return) {
	short sign = 1;

	size_t iterator = 0, accumulator = 0;

	// Negative sign.
	if (__NumberString[iterator] == '-')	
	{	
		sign = -1;
		iterator ++;
	}
	// Positive sign.
	else if (__NumberString[iterator] == '+') 
	{	
		iterator ++;
	}

	while (__NumberString[iterator]) {
		if (is_numeral(__NumberString[iterator])) {
			accumulator *= NUMERICAL_BASE;
			accumulator += fromCharDigit(__NumberString[iterator]);
			iterator ++;
			continue;
		}
		return false;
	}
	* __Return = (int) sign * accumulator;
	return true;
}


// * Oficial implementation?
#define SPEC_IMPL	true


#if SPEC_IMPL

static bool parse_method(const char * __String, int * __ReturnMethod) { // for example*
	if (! assembleNumeral(__String, __ReturnMethod))
	{
		// error msg
		return false;
	}

	if (nin_range(1, 3, *__ReturnMethod))
	{
		// error msg
		return false;
	}
	return true;
}	


static bool parse_qtd(const char * __String, int * __ReturnValue) {
	if (!assembleNumeral(__String, __ReturnValue)) {
		// error msg
		return false;
	}
	return true;
}


static bool parse_file_situation(const char * __String, int * __ReturnValue) {
	if (! assembleNumeral(__String, __ReturnValue)) {
		// error msg
		return false;
	}
	return true;
}


static bool parse_key(const char* __String, int* __ReturnValue) {
	if (!assembleNumeral(__String, __ReturnValue)) {
		// error msg
		return false;
	}
	return true;
}


#define canBeOptionalArg(s)		(s[0] == '-')

static bool inline isPrintingStateFlag(const char * __String) {
	return (__String[0] && __String[0] == '-') && (__String[1] && __String[1] == 'P') && (! __String[2]);
}

#endif


/*
	* SPECS.
*/
int main(int argc, char ** argsv)
{
#if ! SPEC_IMPL
	// Args

	int search_key = 0;
	if (argc > 1) {
		if (! assembleNumeral(argsv[1], & search_key))
			search_key = 0;
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

#else

	// pesquisa <método> <quantidade> <situação> <chave> [-P]
	// qtd: de registros no arq.
	// sit:	1 - ordenado ascendentemente, 2 ordenado descendentemente, 3 - desordenado.
	// -P: printar chaves de pesquisa dos registros do arquivo.

	// Parsing args.

	if (argc < 5) {
		// error msg
		return EXIT_FAILURE;
	}

	// Functional parameters

	bool printing_state = false;
	int method = -1, qtd = -1, situation = -1, key = -1;	// * temp

	int cur_arg = 0;

	for (int i = 1; i < argc; i ++)
	{
		if (canBeOptionalArg(argsv[i]) && isPrintingStateFlag(argsv[i]))
		{
			if (printing_state) {
				// error msg
				// exit?
				return EXIT_FAILURE;
			}
			printing_state = true;
			continue;
		}

		switch (cur_arg) {
		case 0: // method
			if (!parse_method(argsv[i], &method))
			{
				// error msg
				return EXIT_FAILURE;
			} break;
			
		case 1: // qtd
			if (!parse_qtd(argsv[i], &qtd)) {
				// error msg
				return EXIT_FAILURE;
			 } break;

		case 2: // situation
			if (!parse_file_situation(argsv[i], &situation))
			{
				// error msg
				return EXIT_FAILURE;
			} break;

		case 3: // key
			if (!parse_key(argsv[i], &key)) {
				// error msg
				return EXIT_FAILURE;
			} break;

		default:

			// (?)
			;
		}

		cur_arg ++;
	}

	// invariant: all functinal parameters are set.

	printf("%s <%d> <%d> <%d> <%d> [-P:%d]\n",
		argsv[0], method, qtd, situation, key, (int) printing_state);

	return EXIT_SUCCESS;
#endif
}