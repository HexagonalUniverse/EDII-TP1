/*  <src/frame.c>
    
    Where the frame data-structure is defined.
*/


#include "frame.h"
#include <string.h> // for memcpy


inline bool
frame_make(frame_t * _Frame, const size_t _PageSize, page_type _Type) {
    frame_t frame = {
        .page_size =_PageSize,
        .type =     _Type,
        .first =    FRAME_NULL_INDEX,
        .last =     FRAME_NULL_INDEX,
        .size =     0,
        .pages =    0
    };
    * _Frame = frame;
    _Frame ->  pages = calloc(PAGES_PER_FRAME, _PageSize);

    return _Frame -> pages != NULL;
}

inline bool
frame_search_index(const frame_t * _Frame, const uint32_t _Index, uint32_t * _ReturnFrameIndex)
{
    if (isFrameEmpty(_Frame))
        return false;

    for (uint32_t i = _Frame -> first; i <= _Frame -> last; i = (i + 1) % PAGES_PER_FRAME) {
        if (_Index == _Frame -> indexes[i]) {
            *_ReturnFrameIndex = i;
            return true;
        }
    }
    return false;
}

bool frame_remove(frame_t * _Frame) {
    if (isFrameEmpty(_Frame))
        return false;

    /*  Once the frame will no longer hold any element after the remotion, 
        the pointers are reset to FRAME_NULL_INDEX. */
    if (_Frame -> last == _Frame -> first) {
        _Frame -> last = FRAME_NULL_INDEX;
        _Frame -> first = FRAME_NULL_INDEX;
    }

    else {
        // Otherwise an circular increment into the first pointed position of the queue is made.
        _Frame -> first = (_Frame -> first + 1) % PAGES_PER_FRAME;
    }
    _Frame -> size --;

    return true;
}


static finline bool
_frame_last_read_page(uint32_t _PageIndex, frame_t * _Frame, FILE * _Stream) {
    switch (_Frame -> type) {
    case REG_PAGE:
        return read_regpage((REG_STREAM *) _Stream, _PageIndex, & ((regpage_t *) _Frame -> pages)[_Frame -> last]) > 0;
    case B_PAGE:
        return read_bnode((B_STREAM *) _Stream, _PageIndex, & ((b_node *) _Frame -> pages)[_Frame -> last]);
    case BSTAR_PAGE:
        return read_bstar((BSTAR_STREAM *) _Stream, _PageIndex, & ((bstar_node *) _Frame -> pages)[_Frame -> last]);
    case EBST_PAGE:
        return read_ebstnode((EBST_STREAM *) _Stream, _PageIndex, & ((ebst_node *) _Frame -> pages)[_Frame -> last]);
    case ERBT_PAGE:
        return read_erbtnode((ERBT_STREAM *) _Stream, _PageIndex, & ((erbt_node *) _Frame -> pages)[_Frame -> last]);
    default:
        break;
    }
    return false;
}

static finline void *
_frame_page_ptr(frame_t * _Frame, uint32_t _FrameIndex)
{
    switch (_Frame -> type) {
    case REG_PAGE:
        return & ((regpage_t *) _Frame -> pages)[_FrameIndex];
    case B_PAGE:
        return &((b_node *) _Frame->pages)[_FrameIndex];
    case BSTAR_PAGE:
        return &((bstar_node *) _Frame->pages)[_FrameIndex];
    case EBST_PAGE:
        return &((ebst_node *) _Frame->pages)[_FrameIndex];
    case ERBT_PAGE:
        return &((erbt_node *) _Frame->pages)[_FrameIndex];
    default:
        break;
    }

    return NULL;
}

bool frame_add(uint32_t _PageIndex, frame_t * _Frame, FILE * _Stream)
{
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! frame_remove(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }

    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = FRAME_NULL_INDEX;
        _Frame -> last = FRAME_NULL_INDEX;
    }
    
    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);

    // Copying the page into the frame, as well as its index.
    if (! _frame_last_read_page(_PageIndex, _Frame, _Stream))
        return false;

    _Frame -> indexes[_Frame -> last] = _PageIndex;
    _Frame -> size ++;

    return true;
}

static bool 
frame_add_directly(uint32_t _PageIndex, const void * _WritePage, frame_t * _Frame)
{
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! frame_remove(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }

    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = FRAME_NULL_INDEX;
        _Frame -> last = FRAME_NULL_INDEX;
    }

    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);

    // Copying the page into the frame, as well as its index.
    _Frame -> indexes[_Frame -> last] = _PageIndex;
    memcpy(_frame_page_ptr(_Frame, _Frame -> last), _WritePage, _Frame -> page_size);

    _Frame -> indexes[_Frame -> last] = _PageIndex;
    _Frame -> size ++;

    return true;
}



/*

*/
static inline bool
_frame_refresh(frame_t * _Frame, uint32_t _FrameIndex)
{
#if IMPL_LOGGING && DEBUG_FRAME_REFRESH
    DebugPrintf("frame-index: %u, frame-type: %u, frame-size: %u.\n", 
        (unsigned int) _FrameIndex, (unsigned int) _Frame -> type, (unsigned int) _Frame -> size);
    DebugPrintf("page-index: %u.\n", (unsigned int) _Frame -> indexes[_FrameIndex]);
#endif

    if (_FrameIndex == _Frame -> last) {
#if IMPL_LOGGING && DEBUG_FRAME_REFRESH
        DebugPrintf("Page already in last position.\n", NULL);
#endif

        return true;
    }

    void * page_buffer = malloc(_Frame -> page_size);
    if (page_buffer == NULL)
        return false;
    memcpy(page_buffer, _frame_page_ptr(_Frame, _FrameIndex), _Frame -> page_size);

    uint32_t index_buffer = _Frame -> indexes[_FrameIndex];

    for (uint32_t i = _FrameIndex, j; i < _Frame -> last; i = j) {
        j = incr_frame(i);  // j = (i + 1) mod |Frame|.

        DebugPrintf("Frame[%u] <- Frame[%u] (index %u)\n",
            i, j, _Frame -> indexes[j]);

        _Frame -> indexes[i] = _Frame -> indexes[j];
        memcpy(_frame_page_ptr(_Frame, i), _frame_page_ptr(_Frame, j), _Frame -> page_size);
    }

#if IMPL_LOGGING && DEBUG_FRAME_REFRESH
    DebugPrintf("Frame[%u] <- old Frame[%u] (index %u)\n",
        _Frame -> last, _FrameIndex, index_buffer);
#endif

    _Frame -> indexes[_Frame -> last] = index_buffer;
    memcpy(_frame_page_ptr(_Frame, _Frame -> last), page_buffer, _Frame -> page_size);
    free(page_buffer);

    return true;
}

bool frame_retrieve_page(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, void * _ReturnPage) {
    
#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
    DebugPrintf("index: %u\n", (unsigned int) _PageIndex);
#endif

    uint32_t frame_index = 0;
    void * page_ptr = NULL;

    if (frame_search_index(_Frame, _PageIndex, & frame_index))
    {
        page_ptr = _frame_page_ptr(_Frame, frame_index);
        if (page_ptr == NULL)
            return false;

        memcpy(_ReturnPage, page_ptr, _Frame -> page_size);
        return _frame_refresh(_Frame, frame_index);
    }
    if (! frame_add(_PageIndex, _Frame, _Stream)) {

#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
        DebugPrintR("Failed into adding page <%u> to frame.\n", _PageIndex);
#endif

        return false;
    }

#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
    DebugPrintf("Page added to frame. Frame-index: <%u> (last)\n", 
        (unsigned int) _Frame -> last);
#endif

    // TODO: potencially the following block than be extracted a method:
    page_ptr = _frame_page_ptr(_Frame, _Frame -> last);
    if (page_ptr == NULL)
        return false;
    memcpy(_ReturnPage, page_ptr, _Frame -> page_size);

    return true;
}

inline bool frame_retrieve_index(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, uint32_t * _ReturnIndex) {
#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
    DebugPrintf("page index: %u\n", (unsigned int) _PageIndex);
#endif

    uint32_t frame_index = 0;
    if (frame_search_index(_Frame, _PageIndex, & frame_index)) {
        DebugPrintf("(1) frame-index: %u\n", (unsigned int) frame_index);

        // It is refreshed and then, what initially was in frame_index goes to last.
        if (! _frame_refresh(_Frame, frame_index))
            return false;
        * _ReturnIndex = _Frame -> last;
        return true;
    }

    if (! frame_add(_PageIndex, _Frame, _Stream)) {
#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
        DebugPrintR("Failed into adding page <%u> to frame.\n", _PageIndex);
#endif

        return false;
    }

#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
    DebugPrintf("(2) frame-index: %u\n", (unsigned int) _Frame -> last);
#endif

    * _ReturnIndex = _Frame -> last;
    return true;
}

/*

*/
static finline bool
_universal_write_page(FILE * _Stream, uint32_t _PageIndex, const void * _WriteNode, page_type _Type) {
    switch (_Type) {
    case REG_PAGE:
        return write_regpage((REG_STREAM *) _Stream, _PageIndex, (regpage_t *) _WriteNode) > 0;
    case B_PAGE:
        return write_bnode((B_STREAM *) _Stream, _PageIndex, (b_node *) _WriteNode);
    case BSTAR_PAGE:
        return write_bstar((BSTAR_STREAM *) _Stream, _PageIndex, (bstar_node *) _WriteNode);
    case EBST_PAGE:
        return write_ebstnode((EBST_STREAM *) _Stream, _PageIndex, (ebst_node *) _WriteNode);
    case ERBT_PAGE:
        return write_erbtnode((ERBT_STREAM *) _Stream, _PageIndex, (erbt_node *) _WriteNode);
    default:
        break;
    }
    return false;
}

/*  */
inline bool frame_update_page(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, const void * _WritePage)
{
#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
    DebugPrintR("index: %u\n", (unsigned int) _PageIndex);
#endif

    uint32_t frame_index = 0;

    // If the _WritePage (given by _PageIndex) is in the frame, updates then the frame also.
    if (frame_search_index(_Frame, _PageIndex, & frame_index)) {

        memcpy(_frame_page_ptr(_Frame, frame_index), _WritePage, _Frame -> page_size);

        if (! _frame_refresh(_Frame, frame_index)) 
            return false;

    }
    // TODO: Pendent. To add the page in case...
    else {
        if (! frame_add_directly(_PageIndex, _WritePage, _Frame)) {
#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
            DebugPrintR("Failed into adding page <%u> to frame.\n", _PageIndex);
#endif

            return false;
        }

#if IMPL_LOGGING && DEBUG_FRAME_PAGE_MANAGEMENT
        DebugPrintf("Page added to frame. Frame-index: <%u> (last)\n",
            (unsigned int) _Frame -> last);
#endif
    }

    // Updating the page in original stream.
    return _universal_write_page(_Stream, _PageIndex, _WritePage, _Frame -> type);
}






/* Temporary */

static void
_PrintRegistries(const registry_pointer * reg_ptr, const size_t qtd)
{
    putchar('<');
    if (!qtd)
    {
        putchar('>');
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u, ", (unsigned int) reg_ptr[i].key);
    }
    printf("%u>", (unsigned int) reg_ptr[qtd - 1].key);
}

static void
_PrintChildren(const size_t * children, const size_t qtd)
{
    putchar('<');
    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u, ", (unsigned int) children[i]);
    }
    printf("%u>", (unsigned int) children[qtd - 1]);
}

static void
PrintBNode(const b_node * _Node)
{
    printf("\t| [q: %u, leaf: %d, registries key: ", (unsigned) _Node->item_count,
        (int) _Node->is_leaf);

    _PrintRegistries(_Node->reg_ptr, _Node->item_count);

    if (!_Node->is_leaf) {
        printf(", children: ");
        _PrintChildren(_Node->children_ptr, _Node->item_count + 1);
    }
    printf("]\n");
}

/*  */




void show_regpage_frame(const frame_t * _Frame) { // show the Frame pages
    if (isFrameEmpty(_Frame)) {
        return;
    }
    for (uint32_t i = 0; i < PAGES_PER_FRAME; i++) {
        printf("Page %d | (%d)\t", i + 1, _Frame->indexes[i]);
        if (i == _Frame->first) {
            printf("<- first");
        }
        else if (_Frame->last == i) {
            printf("<- last");
        }

        putchar('\n');

        for (int j = 0; j < ITENS_PER_PAGE; j++) {
            printf("Reg %d\n", ((regpage_t *) _Frame->pages)[i].reg[j].key);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

/*  */
void show_bnode_frame(const frame_t * _Frame) {
    if (isFrameEmpty(_Frame))
        return;

    for (uint32_t i = 0; i < PAGES_PER_FRAME; i++) {
        printf("Page %d", i);
        if (i == _Frame->first) {
            printf("\t<- first");
        }
        else if (_Frame->last == i) {
            printf("\t<- last");
        }
        printf("\n%u", _Frame->indexes[i]);
        PrintBNode(&((b_node *) _Frame->pages)[i]);
    }
    printf("\n\n");
}




