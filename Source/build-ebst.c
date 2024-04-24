// <build-ebst.c>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ebst.h"


static bool
doesFileExists(filepath __Filename) {
	FILE * stream_buffer = fopen(__Filename, "rb");
	if (stream_buffer == NULL)
		return false;
	fclose(stream_buffer);
	return true;
}


static char * fileExtension(filepath __FilePath) { // * dynamic
	const size_t length = strlen(__FilePath);

	size_t i;
	for (i = length - 1; i > 0 && __FilePath[i] != '.'; i--);

	char * extension = (char *) malloc(sizeof(char) * (length - i));

	for (size_t j = i; j < length; j++)
		extension[j - i] = __FilePath[j + 1];

	return extension;
}

static finline bool 
streq(const char * __String1, const char * __String2) {
	size_t iterator = 0;
	while (__String1[iterator] && __String2[iterator]) {
		if (__String1[iterator] != __String2[iterator])
			return false;
		iterator ++;
	}
	return __String1[iterator] == __String2[iterator];
}

// * String not-equal.
#define strneq(s1, s2)	(! streq(s1, s2))


/*
	PROGRAM SPECS:
	
	- Inputs a data-file address, and outputs the external data-structure in
	another one.
		
	- Being passed 2 arguments to the program, they will be interpred as
	the input file address and the output file address, respectively.

	- If only one argument is passed, it will be interpreted as the input file.
	The output file will be interrogated in a interface.

	- If none is, then both will be interfaced.

	- Input file should have extension ".bin"; output file must have extension ".ebst"
	for clear identification.
*/
int main(int argc, char ** argsv)
{
	printf("[build-ebst]\n");
	if (argc != 3) {
		fprintf(stderr, "Error: expected 2 arguments but received %d.\n", argc - 1);
		return -1;
	}

	if (!doesFileExists(argsv[1])) {
		fprintf(stderr, "Error: the passed filepath <%s> doesn't match an existing file.\n", argsv[1]);
		return -2;
	}

	char * file_ext_buffer;
	file_ext_buffer = fileExtension(argsv[1]);
	if (file_ext_buffer == NULL) {
		
	}

	if (strneq(file_ext_buffer, "bin")) {
		fprintf(stderr, "Error: The input file doesn't have extension <bin>: it is <%s>.\n", file_ext_buffer);
		free(file_ext_buffer);
		return -3;
	}

	free(file_ext_buffer);
	file_ext_buffer = fileExtension(argsv[2]);
	if (file_ext_buffer == NULL) {

	}

	if (strneq(file_ext_buffer, "ebst")) {
		fprintf(stderr, "Error: The output file doesn't have extension <ebst>: it is <%s>.\n", file_ext_buffer);
		free(file_ext_buffer);
		return -4;
	}

	free(file_ext_buffer);

	if (! assembleEBST_ordered(argsv[1], argsv[2])){
		fprintf(stderr, "Error: the ebst assembling failed.\n");
		return -5;
	}
	
	return EXIT_SUCCESS;
}