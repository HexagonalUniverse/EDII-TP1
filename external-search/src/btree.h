/*  <src/btree.h> 
    
    Where declarations for the B-tree search-engine are placed. */


#ifndef _ES_BTREE_HEADER_
#define _ES_BTREE_HEADER_


#include "searching.h"


#define INSERT_NON_FULL_ITERATIVE   true


/*  */
bool BTree_Build(REG_STREAM *, B_STREAM *);

/*  */
bool BTree_Search(key_t key, REG_STREAM * _InputStream, B_STREAM * _OutputStream, Frame * _Frame, registry_t * target);

/*  (...) */
typedef struct {
    B_STREAM * file_stream;
    uint32_t nodes_qtt;
    Frame frame;
    b_node root;
} B_Builder;

/*  Attempts inserting an element into the B tree. 
    Return success; fails in case of either the inserting key already
    existing in the tree or an atomic frame operation had failed.
    Has undefined state in case of failure. */
bool BTree_insert(const registry_pointer * _reg, B_Builder * _builder);

#endif // _ES_BTREE_HEADER_

