/*	<src/iss.h>
	
	Holds the Indexed Sequential Search searching algorithm. */


#ifndef _ES_ISS_HEADER_
#define _ES_ISS_HEADER_


#include "searching.h"


typedef struct {
	key_t * keys;
	uint32_t length;
} IndexTable;


bool allocateIndexTable(IndexTable * _ReturnTable, const uint32_t _Length);
bool deallocateIndexTable(IndexTable * _Table);
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream);


bool indexedSequencialSearch(const key_t _Key, REG_STREAM * _Stream, IndexTable * _Table, frame_t * _Frame, search_result * _Sr, bool is_ascending);

#endif // _ES_ISS_HEADER_
