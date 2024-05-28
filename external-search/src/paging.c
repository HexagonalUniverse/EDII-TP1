/*  <src/paging.c>

*/

#include "paging.h"



// Pages
// -----

/*  From a data file-stream, reads a single registry page, specified by
    its index. Returns the number of registries read that were read onto the page. */
inline size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.reg.read ++;
#endif

#if IMPL_LOGGING && DEBUG_PAGE_READING
    DebugPrintf("Reading: <%u>\n",
        (unsigned int) _Index);
#endif

    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);
    return fread(_ReturnPage -> reg, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries on the page written. */
inline size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _Page) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.reg.write ++;
#endif
    
#if IMPL_LOGGING && DEBUG_PAGE_WRITING
    DebugPrintf("Writing: <%u>\n",
        (unsigned int) _Index);
#endif

    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);
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
// -----

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool read_bnode(B_STREAM * _Stream, size_t _NodeIndex, b_node * _ReturnNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.b.read ++;
#endif
    
#if IMPL_LOGGING && DEBUG_PAGE_READING
    DebugPrintf("Reading: <%u>\n",
        (unsigned int) _NodeIndex);
#endif

    fseek(_Stream, bnode_pos(_NodeIndex), SEEK_SET);
    return fread(_ReturnNode, sizeof(b_node), 1, _Stream) > 0;
}

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool write_bnode(B_STREAM * _Stream, size_t _NodeIndex, const b_node * _WriteNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.b.write ++;
#endif

#if IMPL_LOGGING && DEBUG_PAGE_WRITING
    DebugPrintf("Writing: <%u>\n",
        (unsigned int) _NodeIndex);
#endif
    
    // DebugPrintf("[%s] %u\n", _Index);
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
bool read_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, erbt_node * _ReturnNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.read++;
#endif

    fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    bool succ = fread(_ReturnNode, sizeof(erbt_node), 1, _Stream) > 0;

#if DEBUG_PAGE_READING 
    DebugPrintG("Reading index <%u>, key: %d\n", 
        (unsigned int) _NodeIndex, _ReturnNode ->reg_ptr.key);
#endif
    return succ;
}

/*  */
bool write_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, const erbt_node * _WriteNode) {
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.write ++;
#endif
    
#if DEBUG_PAGE_WRITING
    DebugPrintG("Writing index <%u>, key: %d\n", (unsigned int) _NodeIndex,
        _WriteNode->reg_ptr.key);
#endif

    fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(erbt_node), 1, _Stream) > 0;
}

inline bool read_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, ebst_node * _Node)
{
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.read++;
#endif

    fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fread(_Node, sizeof(ebst_node), 1, _Stream) > 0;
}

inline bool write_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, const ebst_node * _WriteNode)
{
#ifdef TRANSPARENT_COUNTER
    transparent_counter.ebst.write++;
#endif

    fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fwrite(_WriteNode, sizeof(ebst_node), 1, _Stream) > 0;
}





