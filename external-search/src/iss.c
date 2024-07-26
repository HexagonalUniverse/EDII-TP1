/*	<src/iss.c>

	Holds the Indexed Sequential Search searching algorithm. */


#include "iss.h"


#if ! ISS_PAGING
// Allocate the index table to be used on build function.
bool allocateIndexTable(IndexTable * _ReturnTable, const uint32_t _Length)
{
	_ReturnTable->length = _Length;
	_ReturnTable->keys = (key_t *) malloc(_Length * sizeof(key_t));

	if (_ReturnTable->keys == NULL)
	{
		fprintf(stderr, "couldn't allocate index table...\n");
		_ReturnTable->length = 0;
		return false;
	}
	return true;
}

// Deallocate the index table, used in build function.
bool deallocateIndexTable(IndexTable * _Table)
{
	if (_Table->keys == NULL)
		return false;
	free(_Table->keys);
	return true;
}
#endif // ! ISS_PAGING


static inline void close_index_table(IndexTable * const _Table) { fclose(_Table->file); _Table->file = NULL;  }
static inline boo''l open_index_table_r(IndexTable * const _Table) { return (_Table->file = (INTB_STREAM *) fopen(OUTPUT_ISS_FILENAME, "rb")) != NULL; }

// Build the index table, in order to easy the search.
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream)
{
	#if ISS_PAGING

	// How many entries will there be on the index table.
	const uint64_t length = ceil_div(quantity, REGPAGE_ITENS);

	_ReturnTable->file = (INTB_STREAM *) fopen(OUTPUT_ISS_FILENAME, "w+b");
	if (_ReturnTable->file == NULL)
		return false;

	regpage_t rpage_buffer = { 0 };
	index_page_t ipage_buffer = { 0 };

	uint32_t ipage_index = 0;
	uint64_t i = 0;
	uint64_t reg_index = 0;
	size_t qtt_read;

	while ((reg_index < length) && (qtt_read = read_regpage(_Stream, (uint32_t) reg_index, &rpage_buffer))) {
		reg_index += qtt_read;

		if (i == INDEXPAGE_ARRAY_SIZE) {
			if (! write_indexpage(_ReturnTable->file, ipage_index ++, &ipage_buffer))
				return false;
			
			i = 0;
			ipage_buffer = (index_page_t) { 0 };
		}
		
		ipage_buffer.keys[i ++] = regpage_key(rpage_buffer);
	}

	if (reg_index < length) {
		fprintf(stderr, "(err) %llu < %llu\n", (unsigned long long) reg_index, (unsigned long long) length);
		return false;
	}

	if (i > 0) {
		if (! write_indexpage(_ReturnTable->file, ipage_index, &ipage_buffer))
			return false;
	}
	
	close_index_table(_ReturnTable);
	_ReturnTable->length = length;
	return true;

	#else // ISS_PAGING
	// How many entries will there be on the index table.
    const uint32_t length = ceil_div(quantity, REGPAGE_ITENS);
	
	if (! allocateIndexTable(_ReturnTable, length))
		return false;
	
	regpage_t page_buffer;
	
	uint64_t i = 0;
	while ((i < length) && read_regpage(_Stream, (uint32_t) i, &page_buffer))
		_ReturnTable->keys[i ++] = regpage_key(page_buffer);

	if (i < length) {
		fprintf(stderr, "(err) %llu < %llu\n", (unsigned long long) i, (unsigned long long) length);
		deallocateIndexTable(_ReturnTable);
		return false;
	}

	_ReturnTable->length = length;
	return true;

	#endif // ISS_PAGING
}

// Search for a key on a record page using binary search, which is more efficient than a sequential search.
static inline bool 
_regpage_binarySearch(regpage_t * _regPageArray, key_t key, uint32_t * _ReturnIndex, bool is_ascending) {
    long beg = 0, end = REGPAGE_ITENS - 1;
	long position;
		
	if (is_ascending) {
		while (beg <= end)
		{
			position = ((end - beg) >> 1) + beg;
			if (cmp_eq_search(_regPageArray->reg[position].key, key)){
				*_ReturnIndex = (uint32_t) position;
				return true;
				
			} else if (cmp_bg_search(_regPageArray->reg[position].key, key))
				end = position - 1;
			else
				beg = position + 1;
		}
	}
	else {
		while (beg <= end)
		{
			position = ((end - beg) >> 1) + beg;
			if (cmp_eq_search(_regPageArray->reg[position].key, key)) {
				*_ReturnIndex = (uint32_t) position;
				return true;
			}
			else if (cmp_ls_search(_regPageArray->reg[position].key, key))
				end = position - 1;
			else
				beg = position + 1;
		}
	}

    return false;
}

#if IMPL_LOGGING
#if ISS_PAGING
static void 
printIndexTable(IndexTable * _Table) {
	index_page_t ipage_buffer;
	uint32_t page_index = 0;

	printf("INDEX TABLE:\n");
	size_t qtt_read;
	while ((qtt_read = read_indexpage(_Table->file, page_index++, &ipage_buffer)) > 0) {
		for (size_t i = 0; i < qtt_read; i++)
		{
			printf("%04u ", (unsigned int) ipage_buffer.keys[i]);
		}
		printf("\n");
	}
	printf("\n");
}

#else
static void 
printIndexTable(IndexTable * _Table) {
	printf("INDEX TABLE:\n");
	for (uint32_t i = 0;i < _Table->length; i++){
		printf("|\t%u\n", (unsigned int) _Table->keys[i]);
	}
	printf("\n");
}
#endif
#endif

static inline int64_t indexed_sequential_search(const key_t _Key, const size_t n, key_t keys[static n], bool is_ascending) {
	uint64_t i = 0;		// key iterator.
	if		(is_ascending)	while (cmp_le_search(keys[i], _Key) && (i < n)) i ++;	// Ascending order
	else					while (cmp_be_search(keys[i], _Key) && (i < n)) i ++;	// Descending order
	return i - 1;
}

// The main function.
bool indexedSequencialSearch(const key_t _Key, REG_STREAM * _Stream, 
    IndexTable * _Table, Frame * _Frame, search_result * _Sr, bool is_ascending) 
{
	#if ISS_PAGING

	if (! open_index_table_r(_Table))
		return false;
	
	#if IMPL_LOGGING
		printIndexTable(_Table);
	#endif

	index_page_t ipage_buffer = { 0 };
	uint32_t page_index = 0;
	
	size_t qtt_read;
	int64_t index = -1;
	while ((qtt_read = read_indexpage(_Table->file, page_index ++, &ipage_buffer)) > 0)
	{
		if ((index = indexed_sequential_search(_Key, qtt_read, ipage_buffer.keys, is_ascending)) < 0)
			continue;
	}

	if (index >= 0) {
		uint32_t frame_index;
		uint32_t target_index;

		if (! frame_retrieve_index(_Stream, _Frame, index, &frame_index)) {
			fprintf(stderr, "iss-oa:err2\n");
			close_index_table(_Table);
			return false;
		}

		// Doing the search...
		if (! _regpage_binarySearch(& ((regpage_t *) _Frame->pages)[frame_index], _Key, &target_index, is_ascending))
		{
			fprintf(stderr, "iss-oa:err3\n");
			close_index_table(_Table);
			return false;
		}

		// The target found.
		_Sr->target = ((regpage_t *) _Frame->pages)[frame_index].reg[target_index];
	}
	close_index_table(_Table);
	return (index >= 0);

	#else // ISS_PAGING

	#if IMPL_LOGGING
		printIndexTable(_Table);
	#endif

	uint32_t i = 0, target_index = 0, frame_index = 0;
	
	if		(is_ascending)	while (cmp_le_search(_Table->keys[i], _Key) && (i < _Table->length)) i ++;	// Ascending order
	else					while (cmp_be_search(_Table->keys[i], _Key) && (i < _Table->length)) i ++;	// Descending order
    
	// There was no iteration, return error.
    if (i == 0) {
		fprintf(stderr, "iss:err1\n");
		return false;
    }

	// Attempts retrieving an index inside the frame for a page, given its index. 
	if (! frame_retrieve_index(_Stream, _Frame, i - 1, &frame_index)) {
		fprintf(stderr, "iss-oa:err2\n");
		return false;
	}

	// Doing the search...
	if (! _regpage_binarySearch(& ((regpage_t *) _Frame->pages)[frame_index], _Key, &target_index, is_ascending))
	{
		fprintf(stderr, "iss-oa:err3\n");
		return false;
	}
	
    // The target found.
	_Sr->target = ((regpage_t *) _Frame->pages)[frame_index].reg[target_index];
    return true;
	
	#endif  // ISS_PAGING
}


