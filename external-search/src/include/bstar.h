/*  <src/include/bstar.h>

    Where declarations for the B*-tree search-engine are placed. */


#ifndef _ES_BSTAR_COMMON_HEADER_
#define _ES_BSTAR_COMMON_HEADER_


#include "searching.h"

//  B*-Tree struct
typedef struct {
    BSTAR_STREAM * file_stream; //file pointer
    uint32_t nodes_qtt; //amount of nodes in the tree
    Frame frame; //frame loaded
    bstar_node root; //root of the tree
} BStar_Builder;

// Builds the B*-Tree in the outputstream (B_STREAM)
bool BSTree_Build(REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream, uint64_t _qtt);

// Search for a key in the B*-Tree
bool BSTree_Search(key_t key, REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream, Frame * _Frame, registry_t * target);

/* Attempts inserting an element into the B tree. 
    Return success; fails in case of either the inserting key already
    existing in the tree or an atomic frame operation had failed.
    Has undefined state in case of failure. */
bool BSTree_insert(const registry_pointer * _Reg, BStar_Builder * _bs_builder);



#endif // _ES_BSTAR_COMMON_HEADER_
