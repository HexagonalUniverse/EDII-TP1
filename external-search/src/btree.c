/* <src/btree.c>
    
    Where definitions for the B-tree search-engine are placed. */


#include "btree.h"


/*  Representing the tree. */
#if IMPL_LOGGING

// Prints the registries array of the b-node.
static void
_PrintRegistries(const registry_pointer * reg_ptr, const size_t qtd)
{
    fputc('<', debug_stream);
    if (! qtd)
    {
        fputc('>', debug_stream);
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        fprintf(debug_stream, "%u, ", (unsigned int) reg_ptr[i].key);
    }
    fprintf(debug_stream, "%u>", (unsigned int) reg_ptr[qtd - 1].key);
}

// Prints the children array of the b-node.
static void
_PrintChildren(const uint32_t * children, const size_t qtd) 
{
    fputc('<', debug_stream);
    const size_t qtd_m1 = qtd - 1;

    for (size_t i = 0; i < qtd_m1; i++) {
        fprintf(debug_stream, "%u, ", (unsigned int) children[i]);
    }
    fprintf(debug_stream, " %u>", (unsigned int) children[qtd_m1]);
}

// Prints the b-node, inline.
static void
PrintBNode(const b_node * _Node)
{
    fprintf(debug_stream, "\t[q: %u, leaf: %d, registries key: ", (unsigned) _Node -> item_count,
        (int) _Node -> is_leaf);

    _PrintRegistries(_Node -> reg_ptr, _Node -> item_count);
    
    if (! _Node -> is_leaf) {
        fprintf(debug_stream, ", children: ");
        _PrintChildren(_Node -> children_ptr, _Node -> item_count + 1);
    }
    fprintf(debug_stream, "]\n");
}

// Prints the entire B-tree, delimited above and below by marks "-".
static void
PrintBStream(B_STREAM * _OutputStream, const size_t _HowManyPages)
{
    printDebugSpacing();
    for (size_t i = 0; i < 10; i++)
        fputc('-', debug_stream);
    fprintf(debug_stream, "B-stream:\n");

    fseek(_OutputStream, 0, SEEK_SET);
    
    b_node buffer = { 0 };
    for (size_t i = 0; i < _HowManyPages; i++)
    {
        printDebugSpacing();
        fprintf(debug_stream, "| %u", (unsigned int) i);
        if (!fread(&buffer, sizeof(b_node), 1, _OutputStream))
            break;
        PrintBNode(& buffer);
    }
    printDebugSpacing();
    for (size_t i = 0; i < 20; i++)
        fputc('-', debug_stream);
    fprintf(debug_stream, "\n\n");
}
#endif // IMPL_LOGGING


// B tree methods
// --------------

/*  Splits a full node y into two in the B-tree, y being x's child by the specified index 
    and x an internal node. Updates y and writes the new z node on stream last so-far position.
    Returns success, failing when a frame atomic operation had failed. Undefined state in case
    of failure.

    (The node x has to be updated in the file stream afterwards, externally.) */
static bool BTree_SplitChild(b_node * x, const size_t _Index, B_Builder * _builder) {
    /* Invariants of call:
        . x is a non-full internal node;
        . y is a full node with (2t - 1) keys;
        
        Invariants after (a successfull) call:
        . Both y and z will hold each (t - 1) keys. */

    #if IMPL_LOGGING
        raiseDebug();
    #endif
    
    // The full node to be split.
    b_node y = { 0 };
    if (! frame_retrieve_page(_builder -> file_stream, & _builder -> frame, x -> children_ptr[_Index], & y))
        return false;
    
    const size_t x_within_index = x -> children_ptr[_Index];

    // z is the node that will absorb the great half of y,
    b_node z = { 0 };
    z.is_leaf = y.is_leaf;
    z.item_count = BTREE_MINIMUM_DEGREE - 1;

    // taking its greatest keys,
    for (size_t j = 0; j < BTREE_MINIMUM_DEGREE_m1; j ++)
        z.reg_ptr[j] = y.reg_ptr[j + BTREE_MINIMUM_DEGREE];
    
    // and its corresponding children, if it is the case.
    if (! y.is_leaf) {
        for (size_t j = 0; j < BTREE_MINIMUM_DEGREE; j ++)
            z.children_ptr[j] = y.children_ptr[j + BTREE_MINIMUM_DEGREE];
    }
    y.item_count = BTREE_MINIMUM_DEGREE - 1;
    
    // So we shift x's children to the right,
    for (long j = x -> item_count; j > (long) _Index; j --)
        x -> children_ptr[j + 1] = x -> children_ptr[j];
    
    // and then shift the corresponding keys in x.
    for (long j = x -> item_count - 1; j > ((long) _Index) - 1; j --)
        x -> reg_ptr[j + 1] = x -> reg_ptr[j];
    
    // In the opened spaces, we reference z.
    x -> children_ptr[_Index + 1] = _builder -> nodes_qtt;
    x -> reg_ptr[_Index] = y.reg_ptr[BTREE_MINIMUM_DEGREE - 1];

    // By effect, x have gained a child.
    x -> item_count ++;
    
    // Updates the split child,
    // bnode_write(& y, x_within_index, _BTreeStream -> file_stream);
    if (! frame_update_page(_builder -> file_stream, & _builder -> frame, x_within_index, & y))
        return false;

    // writes the new one.
    if (! frame_update_page(_builder -> file_stream, & _builder -> frame, _builder -> nodes_qtt ++, & z))
        return false;

    // * x is not attempted being written here as an effect of issues of indexing it later.
    
    #if IMPL_LOGGING
        fallDebug();
    #endif

    return true;
}

/*  Treats the case in which the root is a full-node in the tree.
    Returns success, failing in case of failure of a frame atomic operation, essentially.
    Undefined state in case of failure. */
static bool
BTree_SplitRoot(B_Builder * _builder) 
{
    #if IMPL_LOGGING
        raiseDebug();
    #endif
    
    // Will become the new root after splitting the root at this point.
    b_node new_root = { 0 };
    new_root.item_count = 0;

    /*  Writing the old-root as a node in the last position so far
        and setting it as new-root child. */
    new_root.children_ptr[0] = _builder -> nodes_qtt;
    b_node old_root = _builder -> root;
    if (! frame_update_page(_builder -> file_stream, & _builder -> frame, _builder -> nodes_qtt ++, & old_root))
        return false;

    /*  Split-child will split the old-root (with index _builder -> nodes_qtt)
        via new-root. After the split one registry will be raised to it. */
    if (! BTree_SplitChild(& new_root, 0, _builder))
        return false;

    // Propagating the after split alterered new-root information to the builder and the stream.
    if (! frame_update_page(_builder -> file_stream, & _builder -> frame, 0, & new_root))
        return false;
    _builder -> root = new_root;

    #if IMPL_LOGGING
        fallDebug();
    #endif

    return true;
}

/*  A iterative method to operate a binary search in the b-node's registry array. 
    Appropriated for it, since its registries are ordered. 
    Returns the occurrence of the key in it. 
    Used on the build-process, on the "is-leaf" case of insert-non-full. */
static inline bool 
_reg_ptr_key_binary_search(const registry_pointer * _reg_ptr, long length, key_t key) {
    long beg = 0, position = 0, end = length - 1;
    while (beg <= end) {
        position = ((end - beg) / 2) + beg;

        if (cmp_eq_build(_reg_ptr[position].key, key))
            return true;
        else if (cmp_bg_build(_reg_ptr[position].key, key))
            end = position - 1;
        else
            beg = position + 1;
    }
    return false;
}

/*  Inserts a registry-pointer into a node assumed to be non-full
    Can be understood as a base-case for all insertions. 
    Returns success, failing, functionally, in case of the registry's key 
    being attempted inserting is already present, and in other cases as 
    a consequence of failure of the frame operations. 
    Undefined state in case of failure. */
static bool 
BTree_insertNonFull(b_node * x, size_t _XIndex, const registry_pointer * _reg, B_Builder * _builder) {
    /*  Invariants of call:
            x -> item_count < 2t - 1. */
    
    #if IMPL_LOGGING
        raiseDebug();
    #endif
    
#if (! INSERT_NON_FULL_ITERATIVE)
    // Recusive version of the algorithm.
    // ----------------------------------

    // The child node buffer.
    b_node c = { 0 };
    int32_t i = x -> item_count - 1;
    
    // Base-case: reached a leaf node.
    if (x -> is_leaf) {
        // Fails the insertion in case the registry is already present on the node.
        if (_reg_ptr_key_binary_search(x -> reg_ptr, x -> item_count, _reg -> key)) {
            // * For that, a binary search is executed for efficiency.

            #if IMPL_LOGGING
                fallDebug();
            #endif

            return false;
        }

        // Shiffiting the registries that are forward to the position of insertion.
        for (; (i >= 0) && cmp_ls_build(_reg -> key, x -> reg_ptr[i].key); i--)
            x -> reg_ptr[i + 1] = x -> reg_ptr[i];
        i ++;
             
        // Adding the registry to the found position.
        x -> reg_ptr[i] = * _reg;
        x -> item_count ++;
        
        // Updating the leaf-node on the tree structure.
        frame_update_page(_builder -> file_stream, & _builder -> frame, _XIndex, x);

#if IMPL_LOGGING
        DebugPrintf("after base-case <x=%u>:\n", (unsigned int) _XIndex);
#endif
    
    // Traversal case: If the node is not leaf, traverse down the tree.
    } else {

        // Searching for the last registry index such that it comes after the specified one.
        for (; (i >= 0) && cmp_ls_build(_reg -> key, x -> reg_ptr[i].key); i--);
        i ++;


        // Reading the child node indexed from i from Disk.
        frame_retrieve_page(_builder -> file_stream, & _builder -> frame, x -> children_ptr[i], & c);

        if (c.item_count == ((2 * BTREE_MINIMUM_DEGREE) - 1)) {

            BTree_SplitChild(x, i, _builder);
            frame_update_page(_builder->file_stream, &_builder->frame, _XIndex, x);

            // Redirecting the i pointer in case it was forward-shiftted in split-child.
            // By effect, it then points to the exact position at which the next node is pointed at
            // by x.
            if (cmp_bg_build(_reg -> key, x -> reg_ptr[i].key))
                i ++;
            
            // Refreshing the *y*
            frame_retrieve_page(_builder->file_stream, &_builder->frame, x->children_ptr[i], &c);
        }

        // Recursive step
        BTree_insertNonFull(&c, x->children_ptr[i], _reg, _builder);
    }


#else // ! INSERT_NON_FULL_ITERATIVE
    // Iterative version of the algorithm.
    // -----------------------------------

    // The child node buffer.
    b_node c = { 0 };
    c.item_count = 0;
    int32_t i = x -> item_count - 1;

    b_node y = * x;
    uint32_t y_index = _XIndex;

    while (! y.is_leaf)
    {
        c = (b_node) { 0 };

        // TODO: verify here if "_reg -> key" is in reg_ptr array of y.

        for (i = y.item_count - 1; i >= 0 && cmp_ls_build(_reg -> key, y.reg_ptr[i].key); i--);
        i ++;

        // Reading the child node indexed from i from Disk.
        if (! frame_retrieve_page(_builder -> file_stream, & _builder -> frame, y.children_ptr[i], & c))
            return false;

        if (c.item_count == ((2 * BTREE_MINIMUM_DEGREE) - 1)) {
            if (! (BTree_SplitChild(& y, i, _builder) 
                && frame_update_page(_builder -> file_stream, & _builder -> frame, y_index, & y)))
                return false;
            if (_XIndex == y_index)
                * x = y;

            // Redirecting the i pointer in case it was forward-shiftted in split-child.
            // By effect, it then points to the exact position at which the next node is pointed at
            // by y.
            if (cmp_bg_build(_reg -> key, y.reg_ptr[i].key))
                i ++;
            
            // Refreshing the *y*
            if (! frame_retrieve_page(_builder -> file_stream, & _builder -> frame, y.children_ptr[i], & c))
                return false;
        }

        // Recursive step
        y_index = y.children_ptr[i];
        y = c;
    }

    // Fails the insertion in case the registry is already present on the node.
    if (_reg_ptr_key_binary_search(y.reg_ptr, y.item_count, _reg -> key)) {
        // * For that, a binary search is executed for efficiency.
        return false;
    }

    // Shiffiting the registries that are forward to the position of insertion.
    for (i = y.item_count - 1; (i >= 0) && cmp_ls_build(_reg -> key, y.reg_ptr[i].key); i --)
        y.reg_ptr[i + 1] = y.reg_ptr[i];
    i ++;

    // Adding the registry to the found position.
    y.reg_ptr[i] = * _reg;
    y.item_count ++;

    // Updating the leaf-node on the tree structure.
    if (! frame_update_page(_builder -> file_stream, & _builder -> frame, y_index, & y))
        return false;
    if (y_index == _XIndex)
        * x = y;

#endif  // ! INSERT_NON_FULL_ITERATIVE

    #if IMPL_LOGGING
        fallDebug();
    #endif

    return true;
}


bool BTree_insert(const registry_pointer * _reg, B_Builder * _builder)
{
    #if IMPL_LOGGING
        raiseDebug();
    #endif

    /*  If the root has (2t - 1) items, it means that the node is full and needs to be splitted.
        (BTREE_MINIMUM_DEGREE = t) */
    if (_builder -> root.item_count == (2 * BTREE_MINIMUM_DEGREE - 1)) {
        // Splits the current root (on _builder) and assigns the new root to the builder.
        // As the new root is surely not full, after splitted, the insertion by this case can be done.
        if (! BTree_SplitRoot(_builder))
            return false;
    }

    #if IMPL_LOGGING
    bool insert_non_full_success = BTree_insertNonFull(& _builder -> root, 0, _reg, _builder);
    fallDebug();
    return insert_non_full_success;
    #else

    return BTree_insertNonFull(& _builder -> root, 0, _reg, _builder);
    #endif
}

bool BTree_Build(REG_STREAM * _InputStream, B_STREAM * _OutputStream)
{
    #if IMPL_LOGGING
        raiseDebug();
    #endif

    // The handler in the B-tree building process.
    B_Builder b_builder = { 0 };
    b_builder.file_stream = _OutputStream;
    b_builder.root.is_leaf = true;
    b_builder.nodes_qtt = 1;

    if (! frame_make(& b_builder.frame, B_PAGE))
    {
        #if IMPL_LOGGING
            DebugPrintf("bb:err1\n", NULL);
            fallDebug();
        #endif

        return false;
    }

    // The page (node) at which registries will be read onto.
    regpage_t page_buffer = { 0 };

    // The tuple key-pointer that will be passed to the data-structure construction.
    registry_pointer reg_buffer = { 0 };

    size_t page_index = 0, reg_index = 0;   // Iterator variables.
    size_t regs_read = 0;                   // Tracks how many registries does the read page have.

    // Tracks if there happened an error on the last registry insertion.
    bool insert_failure = false;

    /*  While there are no insertion failure and the number of registries read is greater than 0,
        the pages are continuously being read, and each registry in the page being inserted in the tree. */
    while (
        (! insert_failure) &&
        ((regs_read = read_regpage(_InputStream, page_index++, &page_buffer)) > 0))
    {
        // iterating over the registries...
        for (size_t j = 0; j < regs_read; j ++) {
            #if IMPL_LOGGING && DEBUG_REG_INDEX_IN_BUILD
                DebugPrintf("inserting registry [%u]\n", (unsigned int) reg_index);
            #endif

            reg_buffer.key = page_buffer.reg[j].key;
            reg_buffer.original_pos = reg_index++;

            if (! BTree_insert(& reg_buffer, & b_builder)) {
                // In case of insert-failure, the two loops are exited.
                insert_failure = true;
                break;
            }

            #if IMPL_LOGGING
                PrintBStream(b_builder.file_stream, b_builder.nodes_qtt);
            #endif
        }
    }

    // Termination of the building process.
    // * the only dynamic part pending de-allocation being the frame.

    frame_free(& b_builder.frame);

    #if IMPL_LOGGING
        if (insert_failure)
            DebugPrintf("bb:err2\n", NULL);

        fallDebug();
    #endif
    return ! insert_failure;
}

bool BTree_Search(key_t key, REG_STREAM * _RegStream, B_STREAM * _BStream, Frame * _Frame, registry_t * target)
{
    // Tracks the tree node we're at.
    b_node node_buffer = { 0 };
    size_t
        index = 0,  // Tracks the index to the next node.
        i = 0;      // Tracks the index to the pointer to the next node.

    /*  Iterates until a response to the search is given.
        Since 'index' is initialized with 0, at first the
        root node will be pasted into the node-buffer. */
    while (true)
    {
        frame_retrieve_page(_BStream, _Frame, index, & node_buffer);
        
        // printf("index: %u\n", (unsigned int) index);
        // PrintBNode(& node_buffer);
        
        for (i = 0; i < node_buffer.item_count; i ++) {
            if (cmp_eq_search(node_buffer.reg_ptr[i].key, key))
                return search_registry(_RegStream , & node_buffer.reg_ptr[i] , target);

            /*  Since the registries are sorted in a b - node,
                there's no point continuing the search once we have 
                identified that from that point onwards the keys are 
                strictly bigger. */
            else if (cmp_bg_search(node_buffer.reg_ptr[i].key, key))
                break;
        }
        
        /*  In case the registry wasn't found, then we continue searching 
            down the tree. We won't go deeper if the current node is
            already a leaf, in which case the last search was the last
            in the whole process, overall implying in failure. */
        if (node_buffer.is_leaf)
            break;
        index = node_buffer.children_ptr[i];
    }
    
    return false;
}

