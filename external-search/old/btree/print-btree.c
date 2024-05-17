

#include <stdlib.h>
#include <stdbool.h>
#include "btree.h"


#define PRINTFILENAME   "1001stream.out"


/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
bool _DiskRead(b_node * _ReturnNode, size_t _Index, FILE * _BStream) {
    fseek(_BStream, bnode_pos(_Index), SEEK_SET);
    return fread(_ReturnNode, sizeof(b_node), 1 , _BStream) > 0; 
}

static size_t BTree_Depth(FILE * _BStream)
{
    size_t depth = 1;

    b_node node_buffer;

    // Reading the root.
    fseek(_BStream, 0, SEEK_SET);
    fread(& node_buffer, sizeof(b_node), 1, _BStream);

    while (! node_buffer.header.is_leaf) {
        _DiskRead(& node_buffer, node_buffer.children_ptr[0], _BStream);
        depth ++;
    }

    return depth;
}

static inline void _PrintBNode_(const b_node * _Node) {
    printf("[ ");
    for (size_t i = 0; i < _Node -> header.item_count; i++)
        printf("%4u ", (unsigned int) _Node -> reg_ptr[i].key);
    putchar(']');
}

static void _PrintBTreeLevel(const b_node * _Node, FILE * _BStream, size_t level)
{
    if (level == 0) {
        _PrintBNode_(_Node);
        return;
    } else if (_Node -> header.is_leaf) {
        return;
    }

    const size_t how_many_children = _Node -> header.item_count + 1;
    b_node node_buffer;

    for (size_t i = 0; i < how_many_children; i++) {
        _DiskRead(& node_buffer, _Node -> children_ptr[i], _BStream);    
        _PrintBTreeLevel(& node_buffer, _BStream, level - 1);
    }
}


static void
PrintBTree(FILE * _BStream)
{
    b_node node_buffer;
    _DiskRead(& node_buffer, 0, _BStream);

    const size_t tree_depth = BTree_Depth(_BStream);

    for (size_t i = 0; i < tree_depth; i++) {
        _PrintBTreeLevel(& node_buffer, _BStream, i);
        printf("\n|\n");
    }
}


int main(void)
{
    FILE * stream = fopen(PRINTFILENAME, "rb");
    if (! stream)
        return EXIT_FAILURE;
    
    PrintBTree(stream);

    fclose(stream);
    return EXIT_SUCCESS;
}