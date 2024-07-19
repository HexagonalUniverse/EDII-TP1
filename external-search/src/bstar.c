/*  <src/bstar.c>

    B*
*/


#include "bstar.h"


#if IMPL_LOGGING
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

static void
_PrintKeys(const key_t * keys, const size_t qtd)
{
    fputc('<', debug_stream);
    if (! qtd)
    {
        fputc('>', debug_stream);
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        fprintf(debug_stream, "%u*, ", (unsigned int) keys[i]);
    }
    fprintf(debug_stream, "%u*>", (unsigned int) keys[qtd - 1]);
}


static void
_PrintChildren(const uint32_t * children, const size_t qtd) 
{
    fputc('<', debug_stream);
    for (size_t i = 0; i < qtd - 1; i++) {
        fprintf(debug_stream, "%u, ", (unsigned int) children[i]);
    }
    fprintf(debug_stream, "%u>", (unsigned int) children[qtd - 1]);
}

static void
PrintBSNode(const bstar_node * _Node)
{
    if (_Node -> is_leaf) {
        fprintf(debug_stream, "\t| [q: %u, registries keys: ", (unsigned int) _Node -> item_count);
        _PrintRegistries(_Node -> leaf.reg_ptr, _Node -> item_count);
        
    } else {
        fprintf(debug_stream, "\t| [q: %u, registries keys: ", (unsigned int) _Node -> item_count);
        _PrintKeys(_Node -> inner.keys, _Node -> item_count);
        fprintf(debug_stream, ", children: ");
        _PrintChildren(_Node -> inner.children_ptr, _Node -> item_count + 1);
    
    }
    fprintf(debug_stream, "]\n");
}

static void
PrintBSFile(FILE * _OutputStream, size_t _HowManyPages)
{
    printDebugSpacing();
    for (size_t i = 0; i < 20; i++)
        fputc('-', debug_stream);
    fprintf(debug_stream, "\nFILE:\n");

    fseek(_OutputStream, 0, SEEK_SET);
    
    bstar_node buffer;
    for (size_t i = 0; i < _HowManyPages; i++)
    {
        printDebugSpacing();
        fprintf(debug_stream, "%u", (unsigned int) i);
        if (! fread(& buffer, sizeof(bstar_node), 1, _OutputStream))
            break;
        PrintBSNode(& buffer);
    }
    printDebugSpacing();
    fprintf(debug_stream, "\n");
    for (size_t i = 0; i < 20; i++)
        fputc('-', debug_stream);
    fprintf(debug_stream, "\n\n");
}
#endif


// B* Tree methods
// ---------------

/*  TODO */
bool BSTree_SplitChild(bstar_node * x, const size_t _Index, BStar_Builder * _bs_builder)
{
    /*  TODO */

#if IMPL_LOGGING
    raiseDebug();
#endif

    // y is the full node to be split.
    bstar_node y = { 0 };
    
    frame_retrieve_page(_bs_builder->file_stream, & _bs_builder -> frame, x -> inner.children_ptr[_Index], & y);

    const size_t x_within_index = x -> inner.children_ptr[_Index];

    // z is the node that will absorb the great half of y,
    bstar_node z = { 0 };
    z.is_leaf = y.is_leaf;

    z.item_count = BSTREE_MINIMUM_DEGREE_m1;

    if (z.is_leaf)
    {
        z.item_count ++;
        for (size_t j = 0; j < BSTREE_MINIMUM_DEGREE; j ++)
            z.leaf.reg_ptr[j] = y.leaf.reg_ptr[j + BSTREE_MINIMUM_DEGREE_m1];
    }
    else
    // Inner node case 
    {
        for (size_t j = 0; j < BSTREE_MINIMUM_DEGREE_m1; j ++)
            z.inner.keys[j] = y.inner.keys[j + BSTREE_MINIMUM_DEGREE];

        for (size_t j = 0; j < BSTREE_MINIMUM_DEGREE; j ++)
            z.inner.children_ptr[j] = y.inner.children_ptr[j + BSTREE_MINIMUM_DEGREE];
    }

    y.item_count = BSTREE_MINIMUM_DEGREE - 1;
    
    // Invariant: that will always be the case x being an inner-node...
    
    // So we shift x's children to the right,
    for (long j = x -> item_count; j > (long) _Index; j --)
        x -> inner.children_ptr[j + 1] = x -> inner.children_ptr[j];

    const long index_m1 = ((long) _Index) - 1;

    // and then shift the corresponding keys in x.
    for (long j = x -> item_count - 1; j > index_m1; j --)
        x -> inner.keys[j + 1] = x -> inner.keys[j];

    // In the opened spaces, we reference z.
    x -> inner.children_ptr[_Index + 1] = _bs_builder -> nodes_qtt;
    
    if (z.is_leaf)
        x -> inner.keys[_Index] = y.leaf.reg_ptr[BSTREE_MINIMUM_DEGREE - 1].key;
    else
        x -> inner.keys[_Index] = y.inner.keys[BSTREE_MINIMUM_DEGREE - 1];
    

    // By effect, x have gained a child.
    x -> item_count ++;
    
    // Updates the split child,
    frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, x_within_index, & y);
    
    // writes the new one.
    frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, _bs_builder -> nodes_qtt ++, & z);
    
    // x is not attempted being written here as an effect of issues of indexing it.

#if IMPL_LOGGING
    fallDebug();
#endif

    // * Currently it only returns positive, but checking for the success of the disk operations
    // should influence in the overall return of it. If not so, then the return signature shall be 
    // transformed into void.
    return true;
}

//
void 
BSTree_SplitRoot(BStar_Builder * _bs_builder)
{
    // TODO: Make this function safe! void -> bool, verify frame update's and split-child.

    #if IMPL_LOGGING
        raiseDebug();
    #endif

    // TODO: Documentation, as in the B...

    bstar_node new_root = { 0 };
    new_root.item_count = 0;
    new_root.inner.children_ptr[0] = _bs_builder -> nodes_qtt;

    bstar_node old_root = _bs_builder -> root;
    
    frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, _bs_builder -> nodes_qtt ++, & old_root);
    BSTree_SplitChild(& new_root, 0, _bs_builder);
    
    frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, 0, & new_root);

    _bs_builder -> root = new_root;

    #if IMPL_LOGGING
        fallDebug();
    #endif
}


// A iterative method to operate a binary search in a registry array
static inline bool 
_bstar_binarySearch(registry_pointer * _regArray, uint32_t length, key_t key) {
    long beg = 0, position = 0, end = ((long) length) - 1;
    while (beg <= end){
        position = ((end - beg) / 2) + beg;

        if (cmp_eq_build(_regArray[position].key, key))
            return true;
        else if (cmp_bg_build(_regArray[position].key, key))
            end = position - 1;
        else
            beg = position + 1;
    }
    return false;
}

/*  TODO */
bool 
BSTree_insertNonFull(bstar_node * x, const size_t _XIndex, const registry_pointer * _Reg, BStar_Builder * _bs_builder)
{
    // TODO: iterative version, as in B.

    bstar_node c = { 0 };
    int32_t i = x -> item_count - 1; 

    // Base case: leaf node.
    if (x -> is_leaf)
    {

        if (_bstar_binarySearch(x -> leaf.reg_ptr, x -> item_count, _Reg -> key)) {

#if IMPL_LOGGING
            fallDebug();
#endif
            return false;
        }
        
        for (; (i >= 0) && cmp_ls_build(_Reg -> key, x -> leaf.reg_ptr[i].key); i --)
            x -> leaf.reg_ptr[i + 1] = x -> leaf.reg_ptr[i];
        i ++;
    
        x -> leaf.reg_ptr[i] = * _Reg;
        x -> item_count ++;

        frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, _XIndex, x);
    }
    // Traversal case: inner node.
    else {
        for (; i >= 0 && cmp_ls_build(_Reg -> key, x -> inner.keys[i]); i --);
        i ++;
        
        frame_retrieve_page(_bs_builder -> file_stream, & _bs_builder -> frame, x -> inner.children_ptr[i], & c);
        if (c.item_count == (2 * BSTREE_MINIMUM_DEGREE - 1)) {
            BSTree_SplitChild(x, i, _bs_builder);
            frame_update_page(_bs_builder -> file_stream, & _bs_builder -> frame, _XIndex, x);

            if (cmp_bg_build(_Reg -> key, x -> inner.keys[i]))
                i ++;

            frame_retrieve_page(_bs_builder -> file_stream, & _bs_builder -> frame, x -> inner.children_ptr[i], & c);
        }

#if IMPL_LOGGING
        DebugPrintf("(bfr Recursive Step) i = %u\n", (unsigned int) i);
#endif


        BSTree_insertNonFull(& c, x -> inner.children_ptr[i], _Reg, _bs_builder);
    }

#if IMPL_LOGGING
    fallDebug();
#endif

    return true;
}

/*  TODO */
bool 
BSTree_insert(const registry_pointer * _Reg, BStar_Builder * _bs_builder) 
{
#if IMPL_LOGGING
    raiseDebug();
#endif

    /*  TODO */
    if (_bs_builder -> root.item_count == (2 * BSTREE_MINIMUM_DEGREE - 1)) {
        BSTree_SplitRoot(_bs_builder);
    }

#if IMPL_LOGGING
    bool insert_non_full_response = BSTree_insertNonFull(& _bs_builder -> root, 0, _Reg, _bs_builder);
    fallDebug();
    return insert_non_full_response;
#else
    return BSTree_insertNonFull(& _bs_builder -> root, 0, _Reg, _bs_builder);
#endif
} 

bool BSTree_Build(REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream) 
{
#if IMPL_LOGGING
    raiseDebug();
#endif

    // The handler in the B-tree building process.
    BStar_Builder bs_builder = { 0 };
    bs_builder.file_stream = _OutputStream;
    bs_builder.root.is_leaf = true;
    bs_builder.nodes_qtt = 1;

    if (! frame_make(& bs_builder.frame, BSTAR_PAGE))
    {
#if IMPL_LOGGING
        DebugPrintf("bs:err1\n", NULL);
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
        ((regs_read = read_regpage(_InputStream, page_index ++, & page_buffer)) > 0))
    {
        // iterating over the registries...
        for (size_t j = 0; j < regs_read; j ++) {
#if IMPL_LOGGING && DEBUG_REG_INDEX_IN_BUILD
            printf("reg [%u]\n", (unsigned int) reg_index);
#endif

            reg_buffer.key = page_buffer.reg[j].key;
            reg_buffer.original_pos = reg_index ++;

            if (! BSTree_insert(& reg_buffer, & bs_builder)) {
                insert_failure = true;
                break;
            }

#if IMPL_LOGGING && DEBUG_STREAM_AFTER_INSERTION
            PrintBSFile(_OutputStream, bs_builder.nodes_qtt);
#endif
        }
    }

    frame_free(& bs_builder.frame);

#if IMPL_LOGGING
    if (insert_failure)
        DebugPrintf("bs:err2\n", NULL);
    fallDebug();
#endif

    return ! insert_failure;
}


/*  Sopa de macaco. */
bool BSTree_Search(key_t key, REG_STREAM * _RegStream, BSTAR_STREAM * _BStarStream, Frame * _Frame, registry_t * target)
{
    // Tracks the tree node we're at. Initialized to the root.
    bstar_node node_buffer = { 0 };
    frame_retrieve_page(_BStarStream, _Frame, 0, & node_buffer);
    
    // Tracks the index to the pointer to the next node.
    uint8_t i = 0;
    while (! node_buffer.is_leaf) {
        /*  Encontering the next node indirectly via a sequential search. */
        i = 0;
        while (cmp_le_search(node_buffer.inner.keys[i], key) && (i < node_buffer.item_count)) 
            i ++;
        
        frame_retrieve_page(_BStarStream, _Frame, node_buffer.inner.children_ptr[i], & node_buffer);
    }

    /*  Once the target leaf node is found, a binary-search 
        will attempt finding the exact registry pointer stored
        in it; if it does, it proceeds reading the registry
        on the registries-file. */

    // Binary-search parameters.
    long position, beg = 0, end = node_buffer.item_count - 1;

    while (beg <= end) {
        position = ((end - beg) >> 1) + beg;

        if (cmp_eq_search(node_buffer.leaf.reg_ptr[position].key, key))
            /*  Once the registry-pointer is found, it will attempt 
                reading it from the registries-file. If it fails, 
                the whole searching so does. */
            return search_registry(_RegStream, & node_buffer.leaf.reg_ptr[position], target);

        else if (cmp_bg_search(node_buffer.leaf.reg_ptr[position].key, key))
            end = position - 1;

        else
            beg = position + 1;
    }
    return false;
}
