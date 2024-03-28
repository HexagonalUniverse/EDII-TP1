/*
	Data Structures II: Implementation of the pagination-system.

	HexagonDark, 05/03/2024. * Pendent!
*/


// C including.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// C++ including.
#include <iostream>
using namespace std;



/* Conceptual notes.

	Core functionalities of the pagination-system mechanism:
	
	- Address mapping: to determine which page on the secondary-memory a program addresses to,
	and, if it is on the primary-memory, retrieves its frame.

	- Page transfering: as there's the urge for it, transfers pages in and from the primary and
	secondary memories, with sight to the frames.



	Pages-table.
	============

	- A position, generically p, in the pages-table carries the information about the localization
	of the frame in which the page is stored on the primary-memory, p'.
	- Sure enough, if the page is not the primary-memory, an null identity is recognized.



	// definição de uma entrada da tabela de índice das páginas
	typedef struct {
		int posicao;
		int chave;
	} tipoindice;

*/


typedef unsigned char byte;


// How many itens, at its maximum, will hold each page.
byte constexpr itens_per_page = 4;

// How many frames are there to be stored in the pagination-system.
byte constexpr page_frame_size = 10;


// A type used in the pagination-system databank.
typedef struct {
	char name[16];
	int age;
} item_t;	// For instance holds the name and the age of a person.


// Represents a page in the pagination-system.
typedef struct {
	// How many itens are within the page.
	size_t n_itens;		

	// The page's itens.
	item_t itens[itens_per_page];
} page_t;


// (...)
typedef struct {
	int key;
	int address;
} page_address_t;


// The key of the page is simply the key of first one by its itens.
#define page_key(p)	(p.itens[0].age)

#define page_last(p) (p.itens[p.n_itens - 1].age)


static inline void PrintPage(page_t p) {
	printf("{ ");
	for (size_t i = 0; i < p.n_itens; i++)
		printf("%u ", (unsigned int)p.itens[i].age);
	printf("} : %u", (unsigned int)page_key(p));
}



// Organizes and handles a pagination-system in a data-base.
class PaginationSystem 
{
private:
	
	// Stores the page-frames.
	page_t frame_stack[page_frame_size];
	size_t frame_size;

	char * data_filename;


	void LoadPagesFromData(void) {
		cout << "\t| [LoadPagesFromData]\n";

		page_t p0; p0.n_itens = 4;

		item_t di = { "Jose", 10 };
		p0.itens[0] = di;

		di.age = 12;
		p0.itens[1] = di;

		di.age = 13;
		p0.itens[2] = di;

		di.age = 14;
		p0.itens[3] = di;

		frame_stack[0] = p0;

		di.age = 16; p0.itens[0] = di;
		di.age = 17; p0.itens[1] = di;
		di.age = 18; p0.itens[2] = di;
		di.age = 20; p0.itens[3] = di;

		frame_stack[1] = p0;

		frame_size = 2;
	}

public:

	PaginationSystem(const char * __datafilename) {
		printf("[PaginationSystem constructor]\n");

		size_t length = strlen(__datafilename);
		data_filename = (char *) malloc(sizeof(char)* (length ++));
		
		size_t iterator = 0;
		while (iterator < length) {
			data_filename[iterator] = __datafilename[iterator];
			iterator++;
		}

		LoadPagesFromData();

		printf("[PaginationSystem initialized]\n");
	}

	~ PaginationSystem(void) {
		printf("[PaginationSystem deconstructor]\n");

		printf("\t| Data filename: <%s>\n", data_filename);
		free(data_filename);
		
		printf("[PaginationSystem deconstructed]\n");
	}


	// Searchs for the page in which a specified item is in. Returns the search success.
	// The page is returned by reference.
	bool SearchItemsPage(const item_t * item, page_t * return_page) const {
		cout << "[Search for\t??";
		printf("\tItem: <%s, %u>]\n", item -> name, (unsigned int) item -> age);

		size_t iterator = 0;
		while (iterator < frame_size && page_key(frame_stack[iterator]) <= item -> age) 
			iterator ++;

		// The item is supposed being on the place (iterator - 1).

		printf("\t> iterator: %u\n", (unsigned int) iterator);


		// If the item is not in any possible frame in the stack, then it is not there.
		if (iterator == 0) {
			cout << "\t> Couldn't find [1]: underpassed\n";
			return false;
		}


		else if (item -> age > page_last(frame_stack[iterator - 1])) {
			cout << "\t> Couldn't find [2]: overpassed\n";
			return false;
		}


		page_t page = frame_stack[iterator - 1];

		printf("\t> Perhaps the item on searching is on the following page:\n\t>\t");
		PrintPage(page);
		printf("\n\n");

		// Sequential search on the found page.
		for (size_t i = 0; i < page.n_itens; i ++)
			if (page.itens[i].age == item -> age) {
				* return_page = page;
				printf("\t> Found!\n");
				return true;
			}

		// (...)

		printf("Final \"false\" return\n");

		return false;
	}



	void PrintFrame(void) const {
		printf("\tFrame:\n");

		for (size_t i = 0; i < frame_size; i++) {
			printf("\t| ");
			PrintPage(frame_stack[i]);
			printf("\n");
		}

		printf("\n\n");
	}


};




static void __TestSearchs(const PaginationSystem * PS) {
	// Underpassing search.

	item_t dummy_item = { "Hebert", 9 };
	page_t dummy_page;

	PS -> SearchItemsPage(&dummy_item, &dummy_page);

	// Overpassing search.

	dummy_item.age = 24;
	PS->SearchItemsPage(&dummy_item, &dummy_page);


	// Right-spot search.
	dummy_item.age = 19;
	PS->SearchItemsPage(&dummy_item, &dummy_page);

	dummy_item.age = 18;
	PS->SearchItemsPage(&dummy_item, &dummy_page);
}


/*
	Pendent.
*/
int main(int argc, char ** argsv) 
{
	PaginationSystem * PS = new PaginationSystem("nothing.bin");

	PS->PrintFrame();
	
	__TestSearchs(PS);

	cout << "Shut-up\n";
	
	// De-allocation.
	delete PS;


	return EXIT_SUCCESS;
}