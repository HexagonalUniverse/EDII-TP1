

#include "bstar.h"



static void
_PrintRegistries(const registry_pointer * reg_ptr, const size_t qtd)
{
    putchar('<');
    if (! qtd)
    {
        putchar('>');
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u, ", (unsigned int) reg_ptr[i].key);
    }
    printf("%u>", (unsigned int) reg_ptr[qtd - 1].key);
}

static void
_PrintKeys(const key_t * keys, const size_t qtd)
{
    putchar('<');
    if (! qtd)
    {
        putchar('>');
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u*, ", (unsigned int) keys[i]);
    }
    printf("%u*>", (unsigned int) keys[qtd - 1]);
}


static void
_PrintChildren(const uint32_t * children, const size_t qtd) 
{
    putchar('<');
    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u, ", (unsigned int) children[i]);
    }
    printf("%u>", (unsigned int) children[qtd - 1]);
}

static void
PrintBSNode(const bstar_node * _Node)
{
    if (_Node -> is_leaf) {
        printf("\t| [q: %u, registries keys: ", (unsigned int) _Node -> item_count);

        _PrintRegistries(_Node -> leaf.reg_ptr, _Node -> item_count);
        
    } else {
        printf("\t| [q: %u, registries keys: ", (unsigned int) _Node -> item_count);
        _PrintKeys(_Node -> inner.keys, _Node -> item_count);
        printf(", children: ");
        _PrintChildren(_Node -> inner.children_ptr, _Node -> item_count + 1);
    
    }
    printf("]\n");
}

static void
PrintBSFile(FILE * _OutputStream, size_t _HowManyPages)
{
    for (size_t i = 0; i < 20; i++)
        putchar('-');
    printf("\nFILE:\n");
    fseek(_OutputStream, 0, SEEK_SET);
    
    bstar_node buffer;
    for (size_t i = 0; i < _HowManyPages; i++)
    {
        printf("%u", (unsigned int) i);
        fread(& buffer, sizeof(bstar_node), 1, _OutputStream);
        PrintBSNode(& buffer);
    }
    printf("\n");
    for (size_t i = 0; i < 20; i++)
        putchar('-');
    printf("\n\n");
}


bool BSTree_SplitChild(bstar_node * x, const size_t _Index, BStar_Builder * _bs_builder)
{

    // y is the full node to be split.
    // b_node y = retrieveBNode(x -> children_ptr[_Index], _bs_builder);
    bstar_node y = { 0 };
    retrieve_bstar(_bs_builder->file_stream, & _bs_builder -> frame, x -> inner.children_ptr[_Index], & y);

    const size_t x_within_index = x -> inner.children_ptr[_Index];

    // z is the node that will absorb the great half of y,
    bstar_node z = { 0 };
    z.is_leaf = y.is_leaf;

    z.item_count = BTREE_MINIMUM_DEGREE_m1;
    if (z.is_leaf)
        z.item_count ++;

    if (z.is_leaf)
    {
        for (size_t j = 0; j < BTREE_MINIMUM_DEGREE; j ++)
            z.leaf.reg_ptr[j] = y.leaf.reg_ptr[j + BTREE_MINIMUM_DEGREE_m1];
    }
    else
    // Inner node case 
    {
        for (size_t j = 0; j < BTREE_MINIMUM_DEGREE_m1; j ++)
            z.inner.keys[j] = y.inner.keys[j + BTREE_MINIMUM_DEGREE];

        for (size_t j = 0; j < BTREE_MINIMUM_DEGREE; j ++)
            z.inner.children_ptr[j] = y.inner.children_ptr[j + BTREE_MINIMUM_DEGREE];
    }

    y.item_count = BTREE_MINIMUM_DEGREE - 1;
    
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
        x -> inner.keys[_Index] = y.leaf.reg_ptr[BTREE_MINIMUM_DEGREE - 1].key;
    else
        x -> inner.keys[_Index] = y.inner.keys[BTREE_MINIMUM_DEGREE - 1];
    

    // By effect, x have gained a child.
    x -> item_count ++;
    
    // Updates the split child,
    update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, x_within_index, & y);
    
    // writes the new one.
    update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, _bs_builder -> nodes_qtt ++, & z);
    
    // x is not attempted being written here as an effect of issues of indexing it.
    // DiskWrite(x, ,_BTreeStream);

    // * Currently it only returns positive, but checking for the success of the disk operations
    // should influence in the overall return of it. If not so, then the return signature shall be 
    // transformed into void.
    return true;
}

//
bstar_node BSTree_SplitRoot(BStar_Builder * _bs_builder)
{
    bstar_node new_root = { 0 };
    new_root.item_count = 0;
    new_root.inner.children_ptr[0] = _bs_builder -> nodes_qtt;

    bstar_node old_root = _bs_builder -> root;
    
    update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, _bs_builder -> nodes_qtt ++, & old_root);
    BSTree_SplitChild(& new_root, 0, _bs_builder);
    
    update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, 0, & new_root);

    _bs_builder -> root = new_root;
    
    return new_root;
}


// A iterative method to operate a binary search in a registry array
static inline bool 
_bnode_binarySearch(registry_pointer * _regArray, uint32_t length, key_t key) {
    long beg = 0, position = 0, end = length - 1;
    while(beg <= end){
        position = ((end - beg) / 2) + beg;
        if(_regArray[position].key == key)
            return true;
        else if(_regArray[position].key > key)
            end = position - 1;
        else
            beg = position + 1;
    }
    return false;
}


bool BSTree_insertNonFull(bstar_node * x, const size_t _XIndex, const registry_pointer * _Reg, BStar_Builder * _bs_builder)
{
    // DebugFuncMark();

    bstar_node c = { 0 };
    int32_t i = x -> item_count - 1; 

    // Base case: leaf node.
    if (x -> is_leaf)
    {
        if (_bnode_binarySearch(x -> leaf.reg_ptr, x -> item_count, _Reg -> key))
            return false;
        
        for (; i >= 0 && (_Reg -> key < x -> leaf.reg_ptr[i].key); i --)
            x -> leaf.reg_ptr[i + 1] = x -> leaf.reg_ptr[i];
        i ++;
    
        x -> leaf.reg_ptr[i] = * _Reg;
        x -> item_count ++;

        update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, _XIndex, x);
    }
    // Traversal case: inner node.
    else {
        for (; i >= 0 && (_Reg -> key < x -> inner.keys[i]); i --);
        i ++;
        
        retrieve_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, x -> inner.children_ptr[i], & c);
        if (c.item_count == (2 * BTREE_MINIMUM_DEGREE - 1)) {
            BSTree_SplitChild(x, i, _bs_builder);
            update_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, _XIndex, x);

            if (_Reg -> key > x -> inner.keys[i])
                i ++;

            retrieve_bstar(_bs_builder -> file_stream, & _bs_builder -> frame, x -> inner.children_ptr[i], & c);
        }

        DebugPrint("(bfr Recursive Step) i = %u\n", (unsigned int) i);
        
        BSTree_insertNonFull(& c, x -> inner.children_ptr[i], _Reg, _bs_builder);
    }
    return true;
}

bool BSTree_insert(const registry_pointer * _Reg, BStar_Builder * _bs_builder) 
{
    if (_bs_builder -> root.item_count == (2 * BTREE_MINIMUM_DEGREE - 1)) {
        bstar_node s = BSTree_SplitRoot(_bs_builder);

        return BSTree_insertNonFull(& s, 0, _Reg, _bs_builder);
    }
    return BSTree_insertNonFull(& _bs_builder -> root, 0, _Reg, _bs_builder);
} 

/* [] */
bool BSTree_Build(REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream) {
    // The handler in the B-tree building process.
    BStar_Builder bs_builder = { 0 };
    bs_builder.file_stream = _OutputStream;
    bs_builder.root.is_leaf = true;
    bs_builder.nodes_qtt = 1;

    if (! makeFrame(& bs_builder.frame, sizeof(bstar_node)))
    {
        // bb:err1
        printf("bs:err1\n");
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
            printf("reg [%u]\n", (unsigned int) reg_index);

            reg_buffer.key = page_buffer.reg[j].key;
            reg_buffer.original_pos = reg_index ++;

            if (! BSTree_insert(& reg_buffer, & bs_builder)) {
                printf("\t\t"); DebugPrint("Failure", NULL);
                return false;
            }

            PrintBSFile(_OutputStream, bs_builder.nodes_qtt);
        }
    }

    printf("regs_read: %u\n", (unsigned int) regs_read);

    return true;
}


static bool _SearchPage(const registry_pointer * _RegistryPointer, FILE * _Stream, registry_t * _ReturnRegistry)
{   
    regpage_t page = { 0 };
    const size_t page_index = _RegistryPointer -> original_pos / ITENS_PER_PAGE;
    if (! read_regpage(_Stream, page_index, & page)) 
        return false;

    * _ReturnRegistry = page.reg[_RegistryPointer -> original_pos % ITENS_PER_PAGE];
    printf("KEY FOUND! pointer at or.file: %u\n", (unsigned int) _RegistryPointer ->original_pos);
    return true;
}


bool BSTree_Search(key_t key, REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream, frame_t * _Frame, registry_t * target)
{
    bstar_node node_buffer = { 0 }; uint8_t i = 0;
    retrieve_bstar(_OutputStream, _Frame, 0, & node_buffer);
    
    while (! node_buffer.is_leaf) {
        for (i = 0; i < node_buffer.item_count; i ++) { 
            printf("node_buffer.inner.keys[%u] = %d >= %d\n",
                (unsigned int) i, node_buffer.inner.keys[i], key);
            if (node_buffer.inner.keys[i] >= key)
            {
                break; 
            }
        }
        printf("> page <%u>\n", node_buffer.inner.children_ptr[i]);
        retrieve_bstar(_OutputStream, _Frame, node_buffer.inner.children_ptr[i], & node_buffer);
    }
    PrintBSNode(& node_buffer);

    long position, beg = 0, end = node_buffer.item_count - 1;
    while (beg <= end) {
        position = ((end - beg) >> 1) + beg;
        if (node_buffer.leaf.reg_ptr[position].key == key)
            return _SearchPage(& node_buffer.leaf.reg_ptr[position], _InputStream, target);
        else if(node_buffer.leaf.reg_ptr[position].key > key)
            end = position - 1;
        else
            beg = position + 1;
    }
    return false;
}
