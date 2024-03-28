#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Common defs
// -----------

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


// * Temporarily
typedef struct {
	int size;
} data_file_header;



static void TestBinaryFileReading(void)
{
	FILE * datafile = fopen("nothing.bin", "rb");

	data_file_header header;
	fread(& header, sizeof(data_file_header), 1, datafile);
	if (ferror(datafile) != 0) {
		printf("Error (1)\n");
		return;
	}

	printf("Read size: %d\n", header.size);
	
	registry_t dummy_reg;
	for (int i = 0; i < header.size; i ++)
	{
		fread(& dummy_reg, sizeof(registry_t), 1, datafile);
		if (ferror(datafile) != 0) {
			printf("Error (2)\n");
			return;
		}

		
		printf("Reg. Key: %d; data_1: %ld\n", dummy_reg.key, dummy_reg.data_1);
		printf("\tdata_2: %s\n", dummy_reg.data_2);
	}

	printf("Successfully read file\n");
	fclose(datafile);
}





int main(void)
{
	TestBinaryFileReading();
	return EXIT_SUCCESS;
}