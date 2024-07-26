/*	<src/include/iss.h>
	
	Holds the Indexed Sequential Search searching algorithm. */


#ifndef _ES_ISS_HEADER_
#define _ES_ISS_HEADER_


#include "searching.h"


#if ! defined(ISS_PAGING)
	/*	Toggles the paging for the ISS. */
	#define ISS_PAGING	false

#endif // defined(ISS_PAGING)


#if ISS_PAGING
	
	typedef struct {
		INTB_STREAM * file;
		size_t length;
	} IndexTable;

#else // ISS_PAGING

	typedef struct {
		key_t * keys;
		size_t length;
	} IndexTable;

	// Allocate the index table to be used on build function.
	bool allocateIndexTable(IndexTable * _ReturnTable, const uint32_t _Length);

	// Deallocate the index table, used in build function.
	bool deallocateIndexTable(IndexTable * _Table);

#endif // ! ISS_PAGING

// Build the index table, in order to easy the search.
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream);

// The main function.
bool indexedSequencialSearch(const key_t _Key, REG_STREAM * _Stream, IndexTable * _Table, Frame * _Frame, search_result * _Sr, bool is_ascending);

#endif // _ES_ISS_HEADER_
