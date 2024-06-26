/*	<src/iss.c>

*/


#include "iss.h"





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


bool deallocateIndexTable(IndexTable * _Table)
{
	if (_Table->keys == NULL)
		return false;
	free(_Table->keys);
	return true;
}





/*	*/
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream)
{
	// How many entries will there be on the index table.
    const uint32_t length = ceil_div(quantity, ITENS_PER_PAGE);
	
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
}

/*	*/
static inline bool 
_regpage_binarySearch(regpage_t * _regPageArray, key_t key, uint32_t * _ReturnIndex, bool is_ascending) {
    long beg = 0, end = ITENS_PER_PAGE - 1;
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
static void 
printIndexTable(IndexTable * _Table){
	printf("INDEX TABLE:\n");
	for (uint32_t i = 0;i < _Table->length; i++){
		printf("|\t%u\n", (unsigned int) _Table->keys[i]);
	}
	printf("\n");
}
#endif


/*  */
bool indexedSequencialSearch(const key_t _Key, REG_STREAM * _Stream, 
    IndexTable * _Table, frame_t * _Frame, search_result * _Sr, bool is_ascending)
{
	#if IMPL_LOGGING
		printIndexTable(_Table);
	#endif

	uint32_t i = 0, target_index = 0, frame_index = 0;
	    
	if (is_ascending)
		while (cmp_le_search(_Table->keys[i], _Key) && (i < _Table->length)) i ++;
	else // Descending order
		while (cmp_be_search(_Table->keys[i], _Key) && (i < _Table->length)) i ++;
    
    if (i == 0) {
		fprintf(stderr, "iss:err1\n");
		return false;
    }

	if (! frame_retrieve_index(_Stream, _Frame, i - 1, & frame_index)) {
		fprintf(stderr, "iss-oa:err2\n");
		return false;
	}

	if (! _regpage_binarySearch(& ((regpage_t *) _Frame->pages)[frame_index], _Key, & target_index, is_ascending))
	{
		fprintf(stderr, "iss-oa:err3\n");
		return false;
	}
    
	_Sr->target = ((regpage_t *) _Frame->pages)[frame_index].reg[target_index];
    return true;
}


