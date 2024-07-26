/*  <src/include/searching.h> 
	
	The entry for the searching methods. 
	
	Includes here the pagination system for them and defines the
	search responses and the search-result. */


#ifndef _ES_SEARCHING_HEADER_
#define _ES_SEARCHING_HEADER_


#include "frame.h"


// Searching
// ---------

/*	Stores the result of searching processes. */
typedef struct {
	// The return registry in the search.
	registry_t target;
	// ^ 6016 [bytes], aligning by 8.

	struct {
		bool success : 1;
	}; // flags

	/*	Holds on temporal measures about the application of searching. */
	struct {
		/*	The time it takes to construct the data-structures in order
			to do the search. */
		double construction_time;

		/*	The time the search itself takes to completion. */
		double search_time;
	} measures;
} search_result;	// Occupies 6040 [bytes]. Allign by 8 [bytes].


typedef enum {
	// Sinalizes the search went correctly and key holder could be found.
	SEARCH_SUCCESS,

	// Sinalizes the key holder couldn't be found in the search.
	SEARCH_FAILURE,

	/*	Spec. errors */

	// Specification contradiction in case an unordered file is presented to the ISS search-engine.
	_SE_UNORDERED_ISS,


	/*	File stream errors */

	_SE_REGDATAFILE,
	_SE_BFILE,
	_SE_BSTARFILE,
	_SE_EBST_FILE,
	_SE_ERBT_FILE,

	/*	Data-structures errors */

	_SE_MAKEFRAME,
	_SE_INDEXTABLE,
	_SE_BBUILD,
	_SE_ERBTBUILD,
	_SE_EBSTMRTBUILD,
	_SE_BSTARBUILD

} SEARCH_RESPONSE;



#endif // _ES_SEARCHING_HEADER_
