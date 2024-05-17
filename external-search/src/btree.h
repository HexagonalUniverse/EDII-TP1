#ifndef _BTREE_HEADER_
#define _BTREE_HEADER_


#include "paging.h"


// * Interface
typedef struct {
    b_node root;
} BTree;





/*  Splits a full node y into two in the B-Tree, y being x's child by the specified index 
    and x an internal node. Updates y and writes the new z node.
    (Does not updates the node x in the file-stream.) */
//bool BTree_SplitChild(b_node * x, const size_t _Index, BTreeStream * _BTreeStream);

/*  */
//b_node BTree_SplitRoot(BTree * tree, BTreeStream * _BTreeStream);

/*  Auxiliary function to insrting into a node assumed to be non-full
    Can be understood as a base-case for all insertions */
//bool BTree_insertNonFull(b_node * x, const size_t _XIndex, const registry_pointer * _reg, BTreeStream * _BTreeStream);

/*  */
bool BTree_Build(FILE *, FILE*);

/*  */
bool BTree_Search(key_t key, BTreeStream * _BTreeStream, FILE * _InputStream, registry_t * target);

#endif // _BTREE_HEADER_

