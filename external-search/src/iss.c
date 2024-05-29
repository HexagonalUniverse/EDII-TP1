/*	<src/iss.c>

*/


#include "iss.h"





bool allocateIndexTable(IndexTable * _ReturnTable, const uint32_t _Length)
{
	_ReturnTable -> length = _Length;
	_ReturnTable -> keys = (key_t *) malloc(_Length * sizeof(key_t));
	if (_ReturnTable -> keys == NULL)
	{
		_ReturnTable -> length = 0;
		return false;
	}
	return true;
}


bool deallocateIndexTable(IndexTable * _Table)
{
	if (_Table -> keys == NULL)
		return false;
	free(_Table -> keys);
	return true;
}





/*	*/
bool buildIndexTable(IndexTable * _ReturnTable, uint64_t quantity, REG_STREAM * _Stream)
{
    const uint32_t length = ceil_div(quantity, ITENS_PER_PAGE);
	
	if (! allocateIndexTable(_ReturnTable, length))
		return false;
	
	regpage_t page_buffer = { 0 };
	
	uint64_t i = 0;
    for (; i < length; i ++)
    {
		if (! fread(& page_buffer, sizeof(regpage_t), 1, _Stream))
			break;
        _ReturnTable -> keys[i] = regpage_key(page_buffer);
    }
	_ReturnTable -> length = length;
	
	if (i < length) {
		deallocateIndexTable(_ReturnTable);
		return false;
	}
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
			position = ((end - beg) / 2) + beg;
			if (_regPageArray -> reg[position].key == key){
				*_ReturnIndex = (uint32_t) position;
				return true;
			} else if (_regPageArray -> reg[position].key > key)
				end = position - 1;
			else
				beg = position + 1;
		}
	}
	else {
		while (beg <= end)
		{
			position = ((end - beg) / 2) + beg;
			if (_regPageArray->reg[position].key == key) {
				*_ReturnIndex = (uint32_t) position;
				return true;
			}
			else if (_regPageArray->reg[position].key < key)
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
	for (uint32_t i = 0;i < _Table -> length; i++){
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
		while (_Table -> keys[i] <= _Key && i < _Table -> length) i ++;
	else // Descending order
		while (_Table -> keys[i] >= _Key && i < _Table -> length) i ++;
    
    if (i == 0) {
		printf("iss:err1\n");
		return false;
    }

	if (! frame_retrieve_index(_Stream, _Frame, i - 1, & frame_index)) {
		printf("iss-oa:err2\n");
		return false;
	}

	show_regpage_frame(_Frame);

	if (! _regpage_binarySearch(& ((regpage_t *) _Frame -> pages)[frame_index], _Key, & target_index, is_ascending))
	{
		printf("iss-oa:err3\n");
		return false;
	}
    
	_Sr -> target = ((regpage_t *) _Frame -> pages)[frame_index].reg[target_index];
    printf("|KEY: %d|STREAM: %p|SEARCH_RESULT: %p|\n", _Key, _Stream, _Sr);

    return true;
}


