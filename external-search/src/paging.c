/*  <src/paging.c>

*/


#define _FILE_OFFSET_BITS   64
#define _LARGEFILE_SOURCE  
#define _LARGEFILE64_SOURCE

#include "paging.h"


/*  "Large fseek". A "fseek" that can handle larger file's size. */
#define l_fseek(_Stream, _Offset, _Origin)   fseeko64(_Stream, _Offset, _Origin)


inline size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage) {
    #if TRANSPARENT_COUNTER
        transparent_counter.reg.read ++;
    #endif
    
    #if IMPL_LOGGING && DEBUG_PAGE_READING
        DebugPrintf("Reading: <%u>\n",
            (unsigned int) _Index);
    #endif

    // Setting the file pointer to the beggining of the indexed page on the file.
    l_fseek(_Stream, regpage_pos(_Index), SEEK_SET);
    return fread(_ReturnPage -> reg, sizeof(registry_t), REGPAGE_ITENS, _Stream);
}

inline size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _Page) {
    #if TRANSPARENT_COUNTER
        transparent_counter.reg.write ++;
    #endif
    
    #if IMPL_LOGGING && DEBUG_PAGE_WRITING
        DebugPrintf("Writing: <%u>\n",
            (unsigned int) _Index);
    #endif

    // Setting the file pointer to the beggining of the indexed page on the file.
    l_fseek(_Stream, regpage_pos(_Index), SEEK_SET);
    return fwrite(_Page -> reg, sizeof(registry_t), REGPAGE_ITENS, _Stream);
}

inline bool search_registry(REG_STREAM * _Stream, const registry_pointer * _Reference, registry_t * _ReturnRegistry) {
    regpage_t page_buffer = { 0 };

    // The page at which the registry lies on the registries-stream.
    const size_t page_index = _Reference -> original_pos / REGPAGE_ITENS;

    if (! read_regpage(_Stream, page_index, & page_buffer))
        return false;

    // Once the page is read, copies the registry in it.
    * _ReturnRegistry = page_buffer.reg[_Reference -> original_pos % REGPAGE_ITENS];
    return true;
}

inline bool read_bnode(B_STREAM * _Stream, size_t _NodeIndex, b_node * _ReturnNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.b.read ++;
    #endif
    
    #if IMPL_LOGGING && DEBUG_PAGE_READING
        DebugPrintf("Reading: <%u>\n",
            (unsigned int) _NodeIndex);
    #endif

    l_fseek(_Stream, bnode_pos(_NodeIndex), SEEK_SET);
    return fread(_ReturnNode, sizeof(b_node), 1, _Stream) > 0;
}

inline bool write_bnode(B_STREAM * _Stream, size_t _NodeIndex, const b_node * _WriteNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.b.write ++;
    #endif

    #if IMPL_LOGGING && DEBUG_PAGE_WRITING
        DebugPrintf("Writing: <%u>\n",
            (unsigned int) _NodeIndex);
    #endif
    
    l_fseek(_Stream, bnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(b_node), 1, _Stream) > 0;
}

inline bool read_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, bstar_node * _ReturnNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.bs.read ++;
    #endif
    
    l_fseek(_Stream, bstarnode_pos(_NodeIndex), SEEK_SET);
    return fread(_ReturnNode, sizeof(bstar_node), 1, _Stream) > 0;
}

inline bool write_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, const bstar_node * _WriteNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.bs.write ++;
    #endif
    
    l_fseek(_Stream, bstarnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(bstar_node), 1, _Stream) > 0;
}

bool read_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, erbt_node * _ReturnNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.erbt.read ++;
    #endif

    l_fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    bool succ = fread(_ReturnNode, sizeof(erbt_node), 1, _Stream) > 0;

    #if DEBUG_PAGE_READING 
        DebugPrintG("Reading index <%u>, key: %d\n", 
            (unsigned int) _NodeIndex, _ReturnNode ->reg_ptr.key);
    #endif
    return succ;
}

bool write_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, const erbt_node * _WriteNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.erbt.write ++;
    #endif
    
    #if DEBUG_PAGE_WRITING
        DebugPrintG("Writing index <%u>, key: %d\n", (unsigned int) _NodeIndex,
            _WriteNode->reg_ptr.key);
    #endif

    l_fseek(_Stream, erbtnode_pos(_NodeIndex), SEEK_SET);
    return fwrite(_WriteNode, sizeof(erbt_node), 1, _Stream) > 0;
}

inline bool read_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, ebst_node * _Node) {
    #if TRANSPARENT_COUNTER
        transparent_counter.ebst.read ++;
    #endif

    l_fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fread(_Node, sizeof(ebst_node), 1, _Stream) > 0;
}

inline bool write_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, const ebst_node * _WriteNode) {
    #if TRANSPARENT_COUNTER
        transparent_counter.ebst.write ++;
    #endif

    l_fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fwrite(_WriteNode, sizeof(ebst_node), 1, _Stream) > 0;
}
