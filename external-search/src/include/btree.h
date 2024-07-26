/*  <src/include/btree.h> 
    
    Where declarations for the B-tree search-engine are placed. */


#ifndef _ES_BTREE_COMMON_HEADER_
#define _ES_BTREE_COMMON_HEADER_


#include "searching.h"

#define INSERT_NON_FULL_ITERATIVE   true

/*  B-Tree struct */
typedef struct {
    B_STREAM * file_stream; //file pointer
    uint32_t nodes_qtt; //amount of nodes in the tree
    Frame frame; //frame loaded
    b_node root; //root of the tree
} B_Builder;

/* Builds the B-Tree in the outputstreamIn (B_STREAM) */
bool BTree_Build(REG_STREAM * _InputStream, B_STREAM * _OutputStream, uint64_t _qtt);

/* Search for a key in the B-Tree */
bool BTree_Search(key_t key, REG_STREAM * _InputStream, B_STREAM * _OutputStream, Frame * _Frame, registry_t * target);

/*  Attempts inserting an element into the B tree. 
    Return success; fails in case of either the inserting key already
    existing in the tree or an atomic frame operation had failed.
    Has undefined state in case of failure. */
bool BTree_insert(const registry_pointer * _reg, B_Builder * _builder);

#endif // _ES_BTREE_COMMON_HEADER_

