

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


bool BSTree_SplitChild(bstar_node * x, const size_t _Index, BTreeStream * _BTreeStream)
{

    // y is the full node to be split.
    // b_node y = retrieveBNode(x -> children_ptr[_Index], _BTreeStream);
    bstar_node y = { 0 };
    bstar_read(& y, x -> inner.children_ptr[_Index], _BTreeStream -> file_stream);

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
    x -> inner.children_ptr[_Index + 1] = _BTreeStream -> qtd_nodes;

    if (z.is_leaf)
        x -> inner.keys[_Index] = y.leaf.reg_ptr[BTREE_MINIMUM_DEGREE - 1].key;
    else
        x -> inner.keys[_Index] = y.inner.keys[BTREE_MINIMUM_DEGREE - 1];
    

    // By effect, x have gained a child.
    x -> item_count ++;
    
    // Updates the split child,
    bstar_write(& y, x_within_index, _BTreeStream -> file_stream);
    
    // writes the new one.
    bstar_write(& z, _BTreeStream -> qtd_nodes ++, _BTreeStream ->file_stream);
    
    // x is not attempted being written here as an effect of issues of indexing it.
    // DiskWrite(x, ,_BTreeStream);

    // * Currently it only returns positive, but checking for the success of the disk operations
    // should influence in the overall return of it. If not so, then the return signature shall be 
    // transformed into void.
    return true;
}


bstar_node BSTree_SplitRoot(BSTree * _Tree, BTreeStream * _BTreeStream)
{
    DebugFuncMark();

    bstar_node new_root = { 0 };
    new_root.item_count = 0;
    new_root.inner.children_ptr[0] = _BTreeStream -> qtd_nodes;

    bstar_node old_root = _Tree -> root;
    
    bstar_write(& old_root, _BTreeStream -> qtd_nodes ++, _BTreeStream ->file_stream);
    BSTree_SplitChild(& new_root, 0, _BTreeStream);
    
    bstar_write(& new_root, 0, _BTreeStream ->file_stream);

    _Tree -> root = new_root;
    
    // PrintBSFile(_BTreeStream -> file_stream, _BTreeStream -> qtd_nodes);

    return new_root;
}


// A iterative method to operate a binary search in a registry array
static inline bool 
_bnode_binarySearch(registry_pointer * _regArray, long beg, long end, key_t key) {
    long position;
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


bool BSTree_insertNonFull(bstar_node * x, const size_t _XIndex, const registry_pointer * _Reg, BTreeStream * _BTreeStream)
{
    // DebugFuncMark();

    bstar_node c = { 0 };
    int32_t i = x -> item_count - 1; 

    // Base case: leaf node.
    if (x -> is_leaf)
    {
        if (_bnode_binarySearch(x -> leaf.reg_ptr, 0, x -> item_count, _Reg -> key))
            return false;
        
        for (; i >= 0 && (_Reg -> key < x -> leaf.reg_ptr[i].key); i --)
            x -> leaf.reg_ptr[i + 1] = x -> leaf.reg_ptr[i];
        i ++;
    
        x -> leaf.reg_ptr[i] = * _Reg;
        x -> item_count ++;

        bstar_write(x, _XIndex, _BTreeStream ->file_stream);
    }
    // Traversal case: inner node.
    else {
        for (; i >= 0 && (_Reg -> key < x -> inner.keys[i]); i --);
        i ++;

        bstar_read(& c, x -> inner.children_ptr[i], _BTreeStream -> file_stream);
        if (c.item_count == (2 * BTREE_MINIMUM_DEGREE - 1)) {
            BSTree_SplitChild(x, i, _BTreeStream);
            bstar_write(x, _XIndex, _BTreeStream ->file_stream);

            if (_Reg -> key > x -> inner.keys[i])
                i ++;

            bstar_read(& c, x -> inner.children_ptr[i], _BTreeStream -> file_stream);
        }

        DebugPrint("(bfr Recursive Step) i = %u\n", (unsigned int) i);
        
        BSTree_insertNonFull(& c, x -> inner.children_ptr[i], _Reg, _BTreeStream);
    }
    return true;
}

bool BSTree_insert(const registry_pointer * _Reg, BSTree * _Tree, BTreeStream * _BTreeStream) 
{
    if (_Tree -> root.item_count == (2 * BTREE_MINIMUM_DEGREE - 1)) {
        bstar_node s = BSTree_SplitRoot(_Tree, _BTreeStream);

        return BSTree_insertNonFull(& s, 0, _Reg, _BTreeStream);
    }
    return BSTree_insertNonFull(& _Tree -> root, 0, _Reg, _BTreeStream);
} 

/* [] */
bool BSTree_Build(FILE * _InputStream, FILE * _OutputStream, frame_t * _Frame) {
    // TODO: Ponder the initilziation of stuff and stuff.
    //Initialization of the tree
    BSTree tree = { 0 };
    tree.root.is_leaf = true;
    
    //Initialization of the output file variable
    BTreeStream tree_file = { _OutputStream, 1, _Frame };
    
    //Initialization of the page (node) and the registry pointer
    regpage_t page_buffer = { 0 };
    registry_pointer reg_buffer = { 0 };    
    
    //Initialization of the iterator and the variable that gets the amount of registries read
    size_t iterator = 0, regs_read, reg_pos = 0;
    
    //While the number of registries read is greater than 0, the pages are continuously being read 
    while ((regs_read = read_regpage(_InputStream, iterator ++, & page_buffer)) > 0)
    {
        DebugPrint("Page #%u read.\n", (unsigned int) iterator - 1);
        printf("Page:\n");
        for (size_t i = 0; i < regs_read; i++) {
            printf("|\t<%u>\n", (unsigned int) page_buffer.reg[i].key);
        }
        printf("\n\n");
        
        for (size_t j = 0; j < regs_read; j ++) {
            DebugPrint("Inserting registry #%u: <%u, %u>\n",
                (unsigned int) reg_pos, (unsigned int) reg_buffer.key, (unsigned int) reg_buffer.original_pos);
            fflush(stdout);

            reg_buffer.key = page_buffer.reg[j].key;
            reg_buffer.original_pos = reg_pos++;

            if (! BSTree_insert(& reg_buffer, & tree, & tree_file)) {
                printf("\t\t"); DebugPrint("Failure", NULL);
                return false;
            }

            PrintBSFile(_OutputStream, tree_file.qtd_nodes);
        }
        
        /*
        if (iterator > 3) {
            break;
        }
        */
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

    * _ReturnRegistry = page.reg[_RegistryPointer -> original_pos - ITENS_PER_PAGE * page_index];
    printf("KEY FOUND!\n");
    return true;
}

bool BSTree_Search(key_t key, BTreeStream * _BTreeStream, FILE * _InputStream, registry_t * target)
{
    bstar_node node_buffer = { 0 }; uint8_t i = 0;
    bstar_read(& node_buffer, 0, _BTreeStream -> file_stream);

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
        bstar_read(& node_buffer, node_buffer.inner.children_ptr[i], _BTreeStream -> file_stream);
    }
    PrintBSNode(& node_buffer);

    registry_pointer target_pointer = { 0 };
    if (! _bnode_binarySearch(node_buffer.leaf.reg_ptr, 0, node_buffer.item_count - 1, key))
        return false;
    return _SearchPage(& target_pointer, _InputStream, target);
}
