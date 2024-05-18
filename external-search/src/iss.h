
#ifndef _INDEXED_SEQUENTIAL_SEARCH_HEADER_
#define _INDEXED_SEQUENTIAL_SEARCH_HEADER_


#include "paging.h"


/*	*/
typedef struct {
	key_t * keys;
	uint32_t length;
} IndexTable;


bool allocateIndexTable(IndexTable * _ReturnTable, const uint32_t _Length);
bool deallocateIndexTable(IndexTable * _Table);
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream);


bool indexedSequencialSearch(const key_t _Key, REG_STREAM * _Stream, IndexTable * _Table, frame_t * _Frame, search_result * _Sr, bool is_ascending);

#endif // _INDEXED_SEQUENTIAL_SEARCH_HEADER_
