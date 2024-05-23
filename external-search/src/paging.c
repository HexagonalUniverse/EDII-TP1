//  <paging.c>

#include "paging.h"



// Pages
// -----

/*  From a data file-stream, reads a single registry page, specified by
    its index. Returns the number of registries read that were read onto the page. */
inline size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.reg.read ++;
#endif

    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);

    //
    return fread(_ReturnPage -> reg, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries on the page written. */
inline size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _Page) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.reg.write ++;
#endif
    
    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);

    //
    return fwrite(_Page -> reg, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}

/*  From a data file-stream and a reference pointer, attempts retrieving the entire
    registry data from it. Returns success. 

    TODO: (Analysis) Does the frame fit here? */
inline bool search_registry(REG_STREAM * _Stream, const registry_pointer * _Reference, registry_t * _ReturnRegistry) {
    regpage_t page_buffer = { 0 };

    // The page at which the registry lies on the registries-stream.
    const size_t page_index = _Reference -> original_pos / ITENS_PER_PAGE;

    if (! read_regpage(_Stream, page_index, & page_buffer))
        return false;

    // Once the page is read, copies the registry in it.
    * _ReturnRegistry = page_buffer.reg[_Reference -> original_pos % ITENS_PER_PAGE];
    // printf("KEY FOUND! pointer at or.file: %u\n", (unsigned int) _RegistryPointer->original_pos);
    return true;
}



// BTree
// --------

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool read_bnode(B_STREAM * _Stream, size_t _NodeIndex, b_node * _ReturnNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.b.read ++;
#endif
    
    // DebugPrint("index: %u\n", _Index);
    fseek(_Stream, bnode_pos(_NodeIndex), SEEK_SET);
    return fread(_ReturnNode, sizeof(b_node), 1, _Stream) > 0;
}

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool write_bnode(B_STREAM * _Stream, size_t _NodeIndex, const b_node * _WriteNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.b.write ++;
#endif
    
    // DebugPrint("[%s] %u\n", _Index);
    fseek(_Stream, bnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(b_node), 1, _Stream) > 0;
}



// BStar
// -------

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool read_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, bstar_node * _ReturnNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.bs.read ++;
#endif
    
    fseek(_Stream, bstarnode_pos(_NodeIndex), SEEK_SET);
    return fread(_ReturnNode, sizeof(bstar_node), 1, _Stream) > 0;
}

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool write_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, const bstar_node * _WriteNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.bs.write ++;
#endif
    
    fseek(_Stream, bstarnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(bstar_node), 1, _Stream) > 0;
}



// EBST
// ----


/*  */
bool read_erbtnode(EBST_STREAM * _Stream, size_t _NodeIndex, erbt_node * _ReturnNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.read++;
#endif

    fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    bool succ = fread(_ReturnNode, sizeof(erbt_node), 1, _Stream) > 0;

#if DEBUG_READ_EBSTNODE 
    DebugPrintG("Reading index <%u>, key: %d\n", 
        (unsigned int) _NodeIndex, _ReturnNode ->reg_ptr.key);
#endif
    return succ;
}

/*  */
bool write_erbtnode(EBST_STREAM * _Stream, size_t _NodeIndex, const erbt_node * _WriteNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.write ++;
#endif
    
#if DEBUG_WRITE_EBSTNODE
    DebugPrintG("Writing index <%u>, key: %d\n", (unsigned int) _NodeIndex,
        _WriteNode->reg_ptr.key);
#endif

    fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(erbt_node), 1, _Stream) > 0;
}






/* Temporary */

static void
_PrintRegistries(const registry_pointer * reg_ptr, const size_t qtd)
{
    putchar('<');
    if (! qtd)
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
    printf("\t| [q: %u, leaf: %d, registries key: ", (unsigned) _Node -> item_count, 
        (int) _Node -> is_leaf);

    _PrintRegistries(_Node -> reg_ptr, _Node -> item_count);
    
    if (! _Node -> is_leaf) {
        printf(", children: ");
        _PrintChildren(_Node -> children_ptr, _Node -> item_count + 1);
    }
    printf("]\n");
}

/*  */



// Initializes the initial values of the frame
inline bool
makeFrame(frame_t * _Frame, const size_t _PageSize) { 
    frame_t frame = {
        .first = NULL_INDEX,
        .last = NULL_INDEX,
        .sized = 0,
        .pages = 0
    };
    * _Frame = frame;
    _Frame -> pages = calloc(PAGES_PER_FRAME, _PageSize);

    return _Frame -> pages != NULL;
}

//Remove a page using Circular Queue (in this case, this function is a adaptation of the "unqueue" function)
bool removePage(frame_t * _Frame) {
    if (isFrameEmpty(_Frame))
        return false;

    //
    if (_Frame -> last == _Frame -> first){
        _Frame -> last = NULL_INDEX;
        _Frame -> first = NULL_INDEX;
    }
    //
    else {
        _Frame -> first = (_Frame -> first + 1) % PAGES_PER_FRAME;
    }
    _Frame -> sized --;

    return true;
}

//Add a new page to the frame. Receives the number of the page to be placed
bool addPage_regpage(uint32_t num_page, frame_t * _Frame, FILE * _Stream) {
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! removePage(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }

    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = 0;
        _Frame -> last = 0;
    }

    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);

    // Copying the page into the frame, as well as its index.
    read_regpage(_Stream, num_page, & ((regpage_t *) _Frame -> pages)[_Frame -> last]);
    _Frame -> indexes[_Frame -> last] = num_page;
    
    _Frame -> sized ++;
    return true;
}

// Adds a new page to the frame. Receives the number of the page to be placed
bool addPage_b_node(uint32_t _Index, frame_t * _Frame, B_STREAM *_Stream) {
     
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! removePage(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }
    
    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = 0;
        _Frame -> last = 0;
    }
    
    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);
    
    // Copying the page into the frame, as well as its index.
    read_bnode(_Stream, _Index, & ((b_node *) _Frame -> pages)[_Frame -> last]);
    _Frame -> indexes[_Frame -> last] = _Index; 
    
    _Frame -> sized ++;

    // show_bnode_frame(_Frame);
    return true;
}

// Adds a new page to the frame. Receives the number of the page to be placed
bool addPage_bstar(uint32_t _Index, frame_t * _Frame, BSTAR_STREAM *_Stream) {
     
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! removePage(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }
    
    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = 0;
        _Frame -> last = 0;
    }
    
    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);
    
    // Copying the page into the frame, as well as its index.
    read_bstar(_Stream, _Index, & ((bstar_node *) _Frame -> pages)[_Frame -> last]);
    _Frame -> indexes[_Frame -> last] = _Index; 
    
    _Frame -> sized ++;
    return true;
}



void show_regpage_frame(const frame_t *_Frame) { // show the Frame pages
    if(isFrameEmpty(_Frame)){
        return;
    }
    for (uint32_t i = 0; i < PAGES_PER_FRAME; i++) {
        printf("Page %d | (%d)\t", i + 1, _Frame -> indexes[i]);
        if (i == _Frame -> first) {
            printf("<- first");
        } else if (_Frame -> last == i) {
            printf("<- last");
        }

        putchar('\n');

        for(int j=0; j<ITENS_PER_PAGE; j++){
            printf("Reg %d\n", ((regpage_t *)_Frame -> pages)[i].reg[j].key);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

/*  */
void show_bnode_frame(const frame_t *_Frame) {
    if (isFrameEmpty(_Frame))
        return;
    
    for (uint32_t i = 0; i < PAGES_PER_FRAME; i ++) {
        printf("Page %d", i);
        if (i == _Frame -> first) {
            printf("\t<- first");
        } else if (_Frame -> last == i) {
            printf("\t<- last");
        }
        printf("\n%u", _Frame -> indexes[i]);
        PrintBNode(& ((b_node * ) _Frame -> pages)[i]);
    }
    printf("\n\n");
}

/*  */
inline bool 
searchIndexPageInFrame(const frame_t * _Frame, const uint32_t _Index, uint32_t * _ReturnFrameIndex)
{
    if (isFrameEmpty(_Frame))
        return false;

    for (uint32_t i = _Frame -> first;; i = (i + 1) % PAGES_PER_FRAME) {
        if (_Index == _Frame -> indexes[i]) { 
            * _ReturnFrameIndex = i;
            return true;
        }
        if (i == _Frame -> last)
            break;
    }
    return false;
}


/*  */
inline bool retrieve_regpage(REG_STREAM * _Stream, frame_t * _Frame, size_t _Index, uint32_t * _ReturnIndex) {
    uint32_t frame_index = 0;
    if (searchIndexPageInFrame(_Frame, _Index, & frame_index)) {
        * _ReturnIndex = frame_index;
        return true;
    }

    if(! addPage_regpage(_Index, _Frame, _Stream)) {
        printf("\t>addpage");
        return false;
    }

    * _ReturnIndex = _Frame -> last;
    return true;
}

/*  */
inline bool retrieve_bnode(B_STREAM * _Stream, frame_t * _Frame, size_t _Index, b_node * _ReturnNode)
{
    DebugPrintR("index: %u\n", (unsigned int) _Index);

    uint32_t frame_index = 0;
    if (searchIndexPageInFrame(_Frame, _Index, & frame_index))
    {
        printf("\tRefresh:\nframe:%u\t%u", (unsigned int) frame_index, (unsigned int) _Frame -> indexes[frame_index]);
        PrintBNode(& ((b_node *) _Frame -> pages)[frame_index]);
        
        /*
        b_node node_buffer = ((b_node *) _Frame -> pages)[frame_index];
        uint32_t index_buffer =  _Frame -> indexes[frame_index];

        for (uint32_t i = frame_index, j; i < _Frame -> last; i = j) {
            j = incr_frame(i);

            ((b_node *) _Frame -> pages)[i] = ((b_node *) _Frame -> pages)[j];
            _Frame -> indexes[i] = _Frame -> indexes[j];
        }
        
        ((b_node *) _Frame -> pages)[_Frame -> last] = node_buffer;
        _Frame -> indexes[_Frame -> last] = index_buffer;
        
        * _ReturnNode = ((b_node *) _Frame -> pages)[_Frame -> last];
        */

        * _ReturnNode = ((b_node *) _Frame -> pages)[frame_index]; 

        printf("\nReturn node: ");
        PrintBNode(_ReturnNode);
        return true;
    }
    if (! addPage_b_node(_Index, _Frame, _Stream)) {
        printf("\t> addpage\n");
        return false;
    }
    
    printf("\t> pega o último direto msm. last: %u\n", _Frame -> last);
    * _ReturnNode = ((b_node *) _Frame -> pages)[_Frame -> last];
    return true;
}



//

/*  */
bool update_bnode(B_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, const b_node * _WriteNode)
{
    DebugPrintR("index: %u\n", (unsigned int) _NodeIndex);
    
    uint32_t frame_index = 0;

    //If _WriteNode is in the frame, updates the frame also.
    if (searchIndexPageInFrame(_Frame, _NodeIndex, & frame_index)) {

        printf("frame_index: <%u>\n", frame_index);
        ((b_node *) _Frame -> pages)[frame_index] = *_WriteNode;
    }

    //Updating the b_node in file.
    return write_bnode(_Stream, _NodeIndex, _WriteNode);
}

/*  */
inline bool retrieve_bstar(BSTAR_STREAM * _Stream, frame_t * _Frame, size_t _Index, bstar_node * _ReturnNode)
{
    uint32_t frame_index = 0;
    if (searchIndexPageInFrame(_Frame, _Index, & frame_index))
    {
        // TODO: Refresh
        
        * _ReturnNode = ((bstar_node *) _Frame -> pages)[frame_index]; 
        return true;
    }
    if (! addPage_bstar(_Index, _Frame, _Stream)) {
        printf("\t> addpage\n");
        return false;
    }
    
    printf("\t> pega o último direto msm. last: %u\n", _Frame -> last);
    * _ReturnNode = ((bstar_node *) _Frame -> pages)[_Frame -> last];
    return true;
}

/*  */
bool update_bstar(BSTAR_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, const bstar_node * _WriteNode)
{
    DebugPrintR("index: %u\n", (unsigned int) _NodeIndex);
    
    uint32_t frame_index = 0;
    
    //If _WriteNode is in the frame, updates the frame also.
    if (searchIndexPageInFrame(_Frame, _NodeIndex, & frame_index)) {

        printf("frame_index: <%u>\n", frame_index);
        ((bstar_node *) _Frame -> pages)[frame_index] = * _WriteNode;
    }

    //Updating the b_node in file.
    return write_bstar(_Stream, _NodeIndex, _WriteNode);
}


