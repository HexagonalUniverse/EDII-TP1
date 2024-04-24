// <search.c>
// TODO: Documentation


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "searching.h"


#define DATA_FILEPATH	"../Data/nothing.bin"


typedef struct {
	bool deu_certo;
	int x;
} p_functional_parameters; // Program functional parameters



/*
	PROGRAM SPECS:

	-P

	(...)

*/
int main(int argc, char ** argsv)
{

	// pesquisa <método> <quantidade> <situação> <chave> [-P]
	// qtd: de registros no arq.
	// sit:	1 - ordenado ascendentemente, 2 ordenado descendentemente, 3 - desordenado.
	// -P: printar chaves de pesquisa dos registros do arquivo.

	// Parsing args.

	if (argc < 5) {
		// error msg
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}