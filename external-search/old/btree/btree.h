#ifndef _BTREE_HEADER_
#define _BTREE_HEADER_


#include "common.h"
#include <stdio.h>


typedef struct {
    struct {
        uint8_t item_count : 7;                 // (...)
        bool is_leaf : 1;                       // (...)
    } header;

    registry_pointer reg_ptr[ITENS_PER_PAGE];   // (...)
    size_t children_ptr[ITENS_PER_PAGE + 1];    // Children pointer array.
} b_node;


// * Interface
typedef struct {
    b_node root;
} BTree;


// * aeiou
typedef struct {
    FILE * file_stream;
    size_t qtd_nodes;
} BTreeStream;


/*  The minimum degree is here defined as:
        2t = MAX_ITENS_PER_PAGE + 1
        t = (MAX_ITENS_PER_PAGE + 1) / 2 */
#define BTREE_MINIMUM_DEGREE    ((ITENS_PER_PAGE + 1) >> 1)

#define BTREE_MINIMUM_DEGREE_m1 (BTREE_MINIMUM_DEGREE - 1)

// b_node retrieveBNode(size_t node_index, BTreeStream * _BTreeStream);


// Returns the position at which the node given by the passed index is at the 
// B-Tree file stream.
#define bnode_pos(_Index)     (sizeof(b_node) * _Index)

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
bool DiskWrite(const b_node * _Node, size_t _Index, BTreeStream * _BTreeStream);

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
bool DiskRead(b_node * _ReturnNode, size_t _Index, BTreeStream * _BTreeStream);

// From a data file-stream, reads a single registry page, specified by 
// its index. 
// * (OUTDATED) Returns whether the page had been read in its entirety; with its [ITENS_PER_PAGE] itens.
size_t readPage(FILE * _Stream, size_t _Index, page_t * _ReturnPage);

/*  Splits a full node y into two in the B-Tree, y being x's child by the specified index 
    and x an internal node. Updates y and writes the new z node.
    (Does not updates the node x in the file-stream.) */
bool BTree_SplitChild(b_node * x, const size_t _Index, BTreeStream * _BTreeStream);

/*  */
b_node BTree_SplitRoot(BTree * tree, BTreeStream * _BTreeStream);

/*  Auxiliary function to insrting into a node assumed to be non-full
    Can be understood as a base-case for all insertions */
bool BTree_insertNonFull(b_node * x, const size_t _XIndex, const registry_pointer * _reg, BTreeStream * _BTreeStream);

bool BTree_insert(const registry_pointer * _reg, BTree * tree, BTreeStream * _BTreeStream);

bool BTree_Build(FILE * _InputStream, FILE * _OutputStream);

bool BTreeSearch(key_t key, BTreeStream * _BTreeStream, FILE * _InputStream, registry_t * target);



typedef struct {
    // 1 [byte], 1 [byte] alignment.
    struct {
        uint8_t item_count  : 7;
        bool    is_leaf     : 1;
    };
    
    union {
        // 8 * ITENS_PER_PAGE + 4 [bytes], 4 [bytes] alignment.
        struct {
            key_t keys[ITENS_PER_PAGE];
            uint32_t children_ptr[ITENS_PER_PAGE + 1];
        } inner;

        // 8 * ITENS_PER_PAGE [bytes], 4 [bytes] alignment.
        struct {
            registry_pointer reg_ptr[ITENS_PER_PAGE];
        } leaf;
    };

// 8 * ITENS_PER_PAGE + 4 + 1 -> 8 * (ITENS_PER_PAGE + 1) [bytes], 4 [bytes] alignment.
// 48 [bytes].
} bstar_node;


#define bstarnode_pos(_Index)       (_Index * sizeof(bstar_node))

typedef struct {
    bstar_node root;
} BSTree;


bool starDiskWrite(const bstar_node * _Node, size_t _Index, BTreeStream * _BTreeStream);
bool starDiskRead(bstar_node * _ReturnNode, size_t _Index, BTreeStream * _BTreeStream);

#endif // _BTREE_HEADER_