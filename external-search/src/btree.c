// <btree.c>


#include "btree.h"




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
_PrintChildren(const size_t * children, const size_t qtd) 
{
    putchar('<');
    for (size_t i = 0; i < qtd - 1; i++) {
        printf("%u, ", (unsigned int) children[i]);
    }
    printf("%u>", (unsigned int) children[qtd - 1]);
}

static void
PrintBNode(const b_node * _Node)
{
    printf("\t| [q: %u, leaf: %d, registries key: ", (unsigned) _Node -> item_count, 
        (int) _Node -> is_leaf);

    _PrintRegistries(_Node -> reg_ptr, _Node -> item_count);
    
    if (! _Node -> is_leaf) {
        printf(", children: ");
        _PrintChildren(_Node -> children_ptr, _Node -> item_count + 1);
    }
    printf("]\n");
}



static void
PrintFile(FILE * _OutputStream, size_t _HowManyPages)
{
    for (size_t i = 0; i < 20; i++)
        putchar('-');
    printf("\nFILE:\n");
    fseek(_OutputStream, 0, SEEK_SET);
    
    b_node buffer = { 0 };
    for (size_t i = 0; i < _HowManyPages; i++)
    {
        printf("%u", (unsigned int) i);
        fread(& buffer, sizeof(b_node), 1, _OutputStream);
        PrintBNode(& buffer);
    }
    printf("\n");
    for (size_t i = 0; i < 20; i++)
        putchar('-');
    printf("\n\n");
}


// B-Tree Methods
// --------------

/*  Splits a full node y into two in the B-Tree, y being x's child by the specified index 
    and x an internal node. Updates y and writes the new z node.
    (The node x has to be updated in the file stream afterwards, externally.) */
bool BTree_SplitChild(b_node * x, const size_t _Index, B_Builder * _builder) {
    /* Invariants:
        . x is a non-full internal node;
        . y is a full node with (2t - 1) keys;
        . Both y and z will hold each (t - 1) keys.
    */

    // y is the full node to be split.
    b_node y = { 0 };
    
    retrieve_bnode(_builder -> file_stream, & _builder -> frame, x -> children_ptr[_Index], & y);

    printf("y retrieved: ");
    PrintBNode(& y);
    printf("\n\n");

    // show_bnode_frame(_BTreeStream -> frame);

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
    update_bnode(_builder -> file_stream, & _builder -> frame, x_within_index, & y);

    // writes the new one.
    update_bnode(_builder -> file_stream, & _builder -> frame, _builder -> nodes_qtt ++, & z);

    // x is not attempted being written here as an effect of issues of indexing it.

    // * Currently it only returns positive, but checking for the success of the disk operations
    // should influence in the overall return of it. If not so, then the return signature shall be 
    // transformed into void.
    return true;
}

/*  */
b_node BTree_SplitRoot(B_Builder * _builder) 
{
    // * instanciating a new node.
    
    b_node new_root = { 0 };
    new_root.item_count = 0;
    new_root.children_ptr[0] = _builder -> nodes_qtt;

    b_node old_root = _builder -> root;
    update_bnode(_builder -> file_stream, & _builder -> frame, _builder -> nodes_qtt ++, & old_root);

    BTree_SplitChild(& new_root, 0, _builder);
    update_bnode(_builder -> file_stream, & _builder -> frame, 0, & new_root);

    // como quem n quer nada
    _builder -> root = new_root;

    printf("post splitroot:\n");
    PrintFile(_builder->file_stream, _builder->nodes_qtt);

    return new_root;
}   

// A iterative method to operate a binary search in a registry array
static inline bool 
_bnode_binarySearch(registry_pointer * _regArray, long length, key_t key) {
    long beg = 0, position = 0, end = (long) (length - 1);
    while(beg <= end){
        position = ((end - beg) / 2) + beg;
        if (_regArray[position].key == key)
            return true;
        else if(_regArray[position].key > key)
            end = position - 1;
        else
            beg = position + 1;
    }
    return false;
}

/*  Auxiliary function to inserting into a node assumed to be non-full
    Can be understood as a base-case for all insertions. */
bool BTree_insertNonFull(b_node * x, const size_t _XIndex, const registry_pointer * _reg, B_Builder * _builder) {
    /*  Invariants:
            x -> item_count < 2t - 1.

        TODO: (Analysis) 
            Iterative version;
            Resolution in searching for the various indexes;
            Binary search over sequential ones.
    */
    // The child node buffer.
    b_node c = { 0 };
    c.item_count = 0;
     
    int32_t i = x -> item_count - 1;

    // Base-case: reached a leaf node.
    if (x -> is_leaf) {
        // Fails the insertion in case the registry is already present on the node.
        if (_bnode_binarySearch(x -> reg_ptr, x -> item_count, _reg -> key)) {
            // * For that, a binary search is executed for efficiency.
            return false;
        }

        // Shiffiting the registries that are forward to the position of insertion.
        for (; i >= 0 && (_reg -> key < x -> reg_ptr[i].key); i --) {
            x -> reg_ptr[i + 1] = x -> reg_ptr[i];
        }
        i ++;
        
        // Adding the registry to the found position.
        x -> reg_ptr[i] = * _reg;
        x -> item_count ++;
        
        // Updating the leaf-node on the tree structure.
        update_bnode(_builder -> file_stream, & _builder -> frame, _XIndex, x);

        printf("after base-case <x=%u>:\n", (unsigned int) _XIndex);
        PrintBNode(x);

    
    // Traversal case: If the node is not leaf, traverse down the tree.
    } else {
        // TODO: (Analysis) Can it be done with a binary search?
        
        // Searching for the last registry index such that it comes after the specified one.
        for(; i >= 0 && _reg -> key < x -> reg_ptr[i].key; i --);
        i ++;

        // Reading the child node indexed from i from Disk.
        retrieve_bnode(_builder -> file_stream, & _builder -> frame, x -> children_ptr[i], & c);

        if (c.item_count == ((2 * BTREE_MINIMUM_DEGREE) - 1)) {
            
            BTree_SplitChild(x, i, _builder);
            //bnode_write(x, _XIndex, _BTreeStream -> file_stream);
            update_bnode(_builder -> file_stream, & _builder -> frame, _XIndex, x);
            
            // Redirecting the i pointer in case it was forward-shiftted in SplitChild.
            // By effect, it then points to the exact 
            if (_reg-> key > x -> reg_ptr[i].key)
                i ++;
            
            // Refreshing the *y*
            retrieve_bnode(_builder -> file_stream, & _builder -> frame, x -> children_ptr[i], & c);
        }
        
        // Recursive step
        BTree_insertNonFull(& c, x -> children_ptr[i], _reg, _builder);
    }
    return true;
}

/*  (...) */
bool BTree_insert(const registry_pointer * _reg, B_Builder * _builder)
{
    /*  If the root has (2t - 1) items, it means that the node is full and needs to be splitted.
        (BTREE_MINIMUM_DEGREE = t) */
    if (_builder -> root.item_count == 2 * BTREE_MINIMUM_DEGREE - 1) {
        
        // Splits the current root (on _builder) and assigns the new root.
        b_node new_root = BTree_SplitRoot(_builder);
        
        // As the new root is surely not full, after splitted, the insertion by this case can be done.
        return BTree_insertNonFull(& new_root, 0, _reg, _builder);
    }
    return BTree_insertNonFull(& _builder -> root, 0, _reg, _builder);
}


/* [] */
bool BTree_Build(REG_STREAM * _InputStream, B_STREAM * _OutputStream) 
{
    // The handler in the B-tree building process.
    B_Builder b_builder = { 0 };
    b_builder.file_stream = _OutputStream;
    b_builder.root.is_leaf = true;
    b_builder.nodes_qtt = 1;

    if (! makeFrame(& b_builder.frame, sizeof(b_node)))
    {
        // bb:err1
        printf("bb:err1\n");
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

            if (! BTree_insert(& reg_buffer, & b_builder)) {
                // In case of insert-failure, the two loops are exited.
                insert_failure = true;
                break;
            }
            PrintFile(b_builder.file_stream, b_builder.nodes_qtt);
        }
    }

    // Termination of the building process.
    // * the only dynamic part pending de-allocation being the frame.

    freeFrame(b_builder.frame);

    if (insert_failure)
    {
        // bb:err2
        printf("bb:err2\n");
        return false;
    }

    return true;
}


bool _SearchPage(const registry_pointer * _RegistryPointer, FILE * _Stream, registry_t * _ReturnRegistry)
{   
    regpage_t page = { 0 };
    const size_t page_index = _RegistryPointer -> original_pos / ITENS_PER_PAGE;
    if (! read_regpage(_Stream, page_index, & page)) 
        return false;

    * _ReturnRegistry = page.reg[_RegistryPointer -> original_pos - ITENS_PER_PAGE * page_index];
    printf("KEY FOUND!\n");
    return true;
}

#define GOTOVERSION false


/* Sopa de macaco. */
bool BTree_Search(key_t key, FILE * _InputStream, FILE * _OutputStream, frame_t * _Frame, registry_t * target)
{
    b_node node_buffer;
    size_t index = 0, i = 0;
    
#if GOTOVERSION
_BTREESEARCH:
    bnode_read(& node_buffer, index, _BTreeStream -> file_stream);
    
    for (i = 0; i < node_buffer.item_count; i ++) {
        if (node_buffer.reg_ptr[i].key > key)
            goto _BTREESEARCHEXHAUSTION;

        else if (node_buffer.reg_ptr[i].key == key) 
            return _SearchPage(& node_buffer.reg_ptr[i], _InputStream, target);
    }
    
    if (node_buffer.is_leaf)
        goto _BTREESEARCHEXHAUSTION;
    
    index = node_buffer.children_ptr[i];
    goto _BTREESEARCH;
_BTREESEARCHEXHAUSTION:

#else
    while (true) 
    {   
        retrieve_bnode(_OutputStream, _Frame, index, & node_buffer);
    
        printf("index: %u\n", (unsigned int) index);
        PrintBNode(& node_buffer);
        
        for (i = 0; i < node_buffer.item_count; i ++) {
            if (node_buffer.reg_ptr[i].key == key)  {
                return _SearchPage(& node_buffer.reg_ptr[i], _InputStream, target);
            }
            
            else if (node_buffer.reg_ptr[i].key > key)
                break;
        }
        
        if (node_buffer.is_leaf)
            break;

        index = node_buffer.children_ptr[i];
    }
#endif

    return false;
}

