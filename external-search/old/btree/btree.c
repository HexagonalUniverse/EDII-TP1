// <btree.c>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include "common.h"
#include "btree.h"


// * Temporary defs.
// -----------------

#define TEST_FILENAME		"1001stream.bin"
#define TEMPOUT_FILENAME	"1001stream.btree"

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
    printf("\t| [q: %u, leaf: %d, registries key: ", (unsigned) _Node -> header.item_count, 
        (int) _Node -> header.is_leaf);

    _PrintRegistries(_Node -> reg_ptr, _Node -> header.item_count);
    
    if (! _Node -> header.is_leaf) {
        printf(", children: ");
        _PrintChildren(_Node -> children_ptr, _Node -> header.item_count + 1);
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
    
    b_node buffer;
    for (size_t i = 0; i < _HowManyPages; i++)
    {
        fread(& buffer, sizeof(b_node), 1, _OutputStream);
        PrintBNode(& buffer);
    }
    printf("\n");
    for (size_t i = 0; i < 20; i++)
        putchar('-');
    printf("\n\n");
}


// Adaptations
// -----------

uint64_t
large_fseekSet(FILE * _Stream, uint64_t _OffSet)
{
    printf("LARGE FSEEKSET for %" PRIu64 ".\n", _OffSet);
    if (_OffSet < (uint64_t) LONG_MAX)
        return (uint64_t) fseek(_Stream, (long) _OffSet, SEEK_SET);

    uint64_t acc = 0;
    acc += fseek(_Stream, 0, SEEK_SET);
    while (_OffSet > (uint64_t) LONG_MAX) {
        acc += fseek(_Stream, LONG_MAX, SEEK_CUR);
        _OffSet -= LONG_MAX;
    }
    return acc + fseek(_Stream, (long) _OffSet, SEEK_CUR);
}


// B-Tree def.
// -----------



/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool DiskWrite(const b_node * _Node, size_t _Index, BTreeStream * _BTreeStream) {
    printf("\t\t\t");
    DebugPrintY("Writing node (size %u) at index %u.\n",
        (unsigned int) _Node -> header.item_count, (unsigned int) _Index);
    printf("\t\t\t\tWriting:"); PrintBNode(_Node); printf("\n");

    // large_fseekSet(_BTreeStream -> file_stream, bnode_pos64(_Index));
    fseek(_BTreeStream -> file_stream, bnode_pos(_Index), SEEK_SET);
    return fwrite(_Node, sizeof(b_node), 1 , _BTreeStream -> file_stream) > 0; 
}

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool DiskRead(b_node * _ReturnNode, size_t _Index, BTreeStream * _BTreeStream) {
    // large_fseekSet(_BTreeStream -> file_stream, bnode_pos64(_Index));
    fseek(_BTreeStream -> file_stream, bnode_pos(_Index), SEEK_SET);

    size_t qtd_read = fread(_ReturnNode, sizeof(b_node), 1 , _BTreeStream -> file_stream);

    printf("\t\t\t");
    DebugPrintY("Reading node at index %u. Size: %u.\n",
        (unsigned int) _Index, _ReturnNode -> header.item_count);
    printf("\t");
    PrintBNode(_ReturnNode);
    printf("\n");    

    return qtd_read > 0; 
}

/*  From a data file-stream, reads a single registry page, specified by 
    its index. Returns the number of registries read that were read onto the page. */
inline size_t readPage(FILE * _Stream, size_t _Index, page_t * _ReturnPage)
{
    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, (uint64_t) _Index * sizeof(page_t), SEEK_SET);
    return fread(_ReturnPage -> itens, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}


// B-Tree Methods
// --------------

/*  Splits a full node y into two in the B-Tree, y being x's child by the specified index 
    and x an internal node. Updates y and writes the new z node.
    (The node x has to be updated in the file stream afterwards, externally.) */
bool BTree_SplitChild(b_node * x, const size_t _Index, BTreeStream * _BTreeStream) {
    /* Invariants:
        . x is a non-full internal node;
        . y is a full node with (2t - 1) keys;
        . Both y and z will hold each (t - 1) keys.
    */
   
    DebugPrint("Called for _Index = %u\n\t|x| = %u\n", 
        (unsigned int) _Index, (unsigned int) x -> header.item_count);
    
    // y is the full node to be split.
    // b_node y = retrieveBNode(x -> children_ptr[_Index], _BTreeStream);
    b_node y = { 0 };
    DiskRead(& y, x -> children_ptr[_Index], _BTreeStream);
    
    const size_t x_within_index = x -> children_ptr[_Index];

    // z is the node that will absorb the great half of y,
    b_node z = { 0 };
    z.header.is_leaf = y.header.is_leaf;
    z.header.item_count = BTREE_MINIMUM_DEGREE - 1;

    // taking its greatest keys,
    for (size_t j = 0; j < BTREE_MINIMUM_DEGREE_m1; j ++)
        z.reg_ptr[j] = y.reg_ptr[j + BTREE_MINIMUM_DEGREE];
    
    // and its corresponding children, if it is the case.
    if (! y.header.is_leaf) {
        for (size_t j = 0; j < BTREE_MINIMUM_DEGREE; j ++)
            z.children_ptr[j] = y.children_ptr[j + BTREE_MINIMUM_DEGREE];
    }
    y.header.item_count = BTREE_MINIMUM_DEGREE - 1;
    
    
    // So we shift x's children to the right,
    for (long j = x -> header.item_count; j > (long) _Index; j --)
        x -> children_ptr[j + 1] = x -> children_ptr[j];
    
    // and then shift the corresponding keys in x.
    for (long j = x -> header.item_count - 1; j > ((long) _Index) - 1; j --)
        x -> reg_ptr[j + 1] = x -> reg_ptr[j];
    
    // In the opened spaces, we reference z.
    x -> children_ptr[_Index + 1] = _BTreeStream -> qtd_nodes;
    x -> reg_ptr[_Index] = y.reg_ptr[BTREE_MINIMUM_DEGREE - 1];

    // By effect, x have gained a child.
    x -> header.item_count ++;
    
    aec_fg_rgb(255, 100, 100); printf("[%s]\t", __func__); aec_reset();
    // Updates the split child,
    DiskWrite(& y, x_within_index, _BTreeStream);
    
    aec_fg_rgb(255, 100, 100); printf("[%s]\t", __func__); aec_reset();
    // writes the new one.
    DiskWrite(& z, _BTreeStream -> qtd_nodes ++, _BTreeStream);
    
    // x is not attempted being written here as an effect of issues of indexing it.
    // DiskWrite(x, ,_BTreeStream);

    // * Currently it only returns positive, but checking for the success of the disk operations
    // should influence in the overall return of it. If not so, then the return signature shall be 
    // transformed into void.
    return true;
}

/*  */
b_node BTree_SplitRoot(BTree * tree, BTreeStream * _BTreeStream) 
{
    printf("\t");
    DebugFuncMark();
    
    // * instanciating a new node.
    
    b_node new_root = { 0 };
    new_root.header.item_count = 0;
    new_root.children_ptr[0] = _BTreeStream -> qtd_nodes;

    b_node old_root = tree -> root;
    
    aec_fg_rgb(100, 255, 100); printf("\t[%s]\t", __func__); aec_reset();
    DiskWrite(& old_root, _BTreeStream -> qtd_nodes ++, _BTreeStream);

    BTree_SplitChild(& new_root, 0, _BTreeStream);
    
    aec_fg_rgb(100, 255, 100); printf("\t[%s]\t", __func__); aec_reset();
    DiskWrite(& new_root, 0, _BTreeStream);

    // como quem n quer nada
    tree -> root = new_root;

    return new_root;
}

// A iterative method to operate a binary search in a registry array
static inline bool 
_Bnode_binarySearch(registry_pointer * _regArray, long beg, long end, key_t key) {
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

/*  Auxiliary function to insrting into a node assumed to be non-full
    Can be understood as a base-case for all insertions. */
bool BTree_insertNonFull(b_node * x, const size_t _XIndex, const registry_pointer * _reg, BTreeStream * _BTreeStream) {
    /*  Invariants:
            x -> header.item_count < 2t - 1.

        TODO: (Analysis) 
            Iterative version;
            Resolution in searching for the various indexes;
            Binary search over sequential ones.
    */
    printf("\t");
    DebugPrint("x: #%u <size: %u> | ", (unsigned int) _XIndex, (unsigned int) x -> header.item_count);

    // The child node buffer.
    b_node c = { 0 };
    c.header.item_count = 0;

    int i = x -> header.item_count - 1;

    // Base-case: reached a leaf node.
    if (x -> header.is_leaf) {
        printf("Base case\n");

        // Fails the insertion in case the registry is already present on the node.
        if (_Bnode_binarySearch(x -> reg_ptr, 0, x -> header.item_count, _reg -> key)) {
            // * For that, a binary search is executed for efficiency.
            printf("[BTree_insertNonFull]: key already on tree.\n");
            return false;
        }
        
        printf("\t\t>\tx -> reg_ptr[%d] = %u\n",
             (int) i, (unsigned int) x -> reg_ptr[i].key); 


        // Shiffiting the registries that are forward to the position of insertion.
        for (; i >= 0 && (_reg -> key < x -> reg_ptr[i].key); i --) {
            printf("\t{ Shifting %d to %d}\n", i, i + 1);
            x -> reg_ptr[i + 1] = x -> reg_ptr[i];
        }
        i ++;
        
        // Adding the registry to the found position.
        x -> reg_ptr[i] = * _reg;
        // printf("[ESCREVI E SAI CORRENDO] x -> reg_ptr[%u].key = %u\n", (unsigned int) i, (unsigned int) x -> reg_ptr[i].key);
        x -> header.item_count ++;
        
        printf("\t\t>\tX's registries after shift: "); 
        _PrintRegistries(x -> reg_ptr, x -> header.item_count);
        printf("\n");

        // Updating the leaf-node on the tree structure.
        DiskWrite(x, _XIndex, _BTreeStream);
    
    // Traversal case: If the node is not leaf, traverse down the tree.
    } else {
        // TODO: (Analysis) Can it be done with a binary search?
        
        // Searching for the last registry index such that it comes after the specified one.
        for(; i >= 0 && _reg -> key < x -> reg_ptr[i].key; i --);
        i ++;
        
        printf("Traversal case, i: %u\n", (unsigned int) i);
        DebugPrint("X node:\n\t", NULL);
        PrintBNode(x);
        printf("\\n\n");
        
        // Reading the child node indexed from i from Disk.
        DiskRead(& c, x -> children_ptr[i], _BTreeStream);

        if (c.header.item_count == ((2 * BTREE_MINIMUM_DEGREE) - 1)) {

            BTree_SplitChild(x, i, _BTreeStream);
            DiskWrite(x, _XIndex, _BTreeStream);
            
            // Redirecting the i pointer in case it was forward-shiftted in SplitChild.
            // By effect, it then points to the exact 
            if (_reg-> key > x -> reg_ptr[i].key)
                i ++;
            
            // Refreshing the *y*
            DiskRead(& c, x -> children_ptr[i], _BTreeStream);
        }
        
        DebugPrint("(bfr Recursive Step) i = %u\n", (unsigned int) i);

        // Recursive step
        BTree_insertNonFull(& c, x -> children_ptr[i], _reg, _BTreeStream);
    }
    return true;
}

/* */
bool BTree_insert(const registry_pointer * _reg, BTree * tree, BTreeStream * _BTreeStream)
{
    // DebugPrint("Inserting ref-reg <%3u, %3u> | ", (unsigned int) _reg -> key, (unsigned int) _reg -> original_pos);
    
    // If the root has 2*t-1 (BTREE_MINIMUM_DEGREE = t) items, it means that the node is full and needs to be splitted
    if (tree -> root.header.item_count == 2 * BTREE_MINIMUM_DEGREE - 1) {
        
        //The function that splits nodes is called and the new node is called s
        b_node s = BTree_SplitRoot(tree, _BTreeStream);
        
        //After splitted, now the new node surely not full, so the function of normal insertion is called
        return BTree_insertNonFull(& s, 0, _reg, _BTreeStream);
    }
    return BTree_insertNonFull(& tree -> root, 0, _reg, _BTreeStream);
}

/* [] */
bool BTree_Build(FILE * _InputStream, FILE * _OutputStream) {
    // TODO: Ponder the initilziation of stuff and stuff.
    //Initialization of the tree
    BTree tree = { 0 };
    tree.root.header.is_leaf = true;
    
    //Initialization of the output file variable
    BTreeStream tree_file = { _OutputStream, 1 };
    
    //Initialization of the page (node) and the registry pointer
    page_t page_buffer = { 0 };
    registry_pointer reg_buffer = { 0 };    
        
    //Initialization of the iterator and the variable that gets the amount of registries read
    size_t iterator = 0, regs_read, reg_pos = 0;
    
    //While the number of registries read is greater than 0, the pages are continuously being read 
    while ((regs_read = readPage(_InputStream, iterator ++, & page_buffer)) > 0) 
    {
        DebugPrint("Page #%u read.\n", (unsigned int) iterator - 1);
        printf("Page:\n");
        for (size_t i = 0; i < regs_read; i++) {
            printf("|\t<%u>\n", (unsigned int) page_buffer.itens[i].key);
        }
        printf("\n\n");
        
        for (size_t j = 0; j < regs_read; j ++) {
            DebugPrint("Inserting registry #%u: <%u, %u>\n",
                (unsigned int) reg_pos, (unsigned int) reg_buffer.key, (unsigned int) reg_buffer.original_pos);
            
            reg_buffer.key = page_buffer.itens[j].key;
            reg_buffer.original_pos = reg_pos++;

            if (! BTree_insert(& reg_buffer, & tree, & tree_file)) {
                printf("\t\t"); DebugPrint("Failure", NULL);
                return false;
            }

            PrintFile(_OutputStream, tree_file.qtd_nodes);
        }
    }

    printf("regs_read: %u\n", (unsigned int) regs_read);

    return true;
}


bool _Search_Page(const registry_pointer * _RegistryPointer, FILE * _Stream, registry_t * _ReturnRegistry)
{   
    printf("at position: %u\n", (unsigned int) _RegistryPointer -> original_pos);

    page_t page = { 0 };
    const size_t page_index = _RegistryPointer -> original_pos / ITENS_PER_PAGE;
    if (! readPage(_Stream, page_index, & page)) 
        return false;

    * _ReturnRegistry = page.itens[_RegistryPointer -> original_pos - ITENS_PER_PAGE * page_index];
    return true;
}

#define GOTOVERSION false


/* Sopa de macaco. */
bool BTreeSearch(key_t key, BTreeStream * _BTreeStream, FILE * _InputStream, registry_t * target)
{
    b_node node_buffer;
    size_t index = 0, i;
    
#if GOTOVERSION
_BTREESEARCH:
    DiskRead(& node_buffer, index, _BTreeStream);
    
    for (i = 0; i < node_buffer.header.item_count; i ++) {
        if (node_buffer.reg_ptr[i].key > key)
            goto _BTREESEARCHEXHAUSTION;

        else if (node_buffer.reg_ptr[i].key == key) 
            return _Search_Page(& node_buffer.reg_ptr[i], _InputStream, target);
    }
    
    if (node_buffer.header.is_leaf)
        goto _BTREESEARCHEXHAUSTION;
    
    index = node_buffer.children_ptr[i];
    goto _BTREESEARCH;
_BTREESEARCHEXHAUSTION:

#else
    while (true) 
    {
        DiskRead(& node_buffer, index, _BTreeStream);
        
        for (i = 0; i < node_buffer.header.item_count; i ++) {
            if (node_buffer.reg_ptr[i].key == key)  {
                printf("KEY EQUAL\n");
                return _Search_Page(& node_buffer.reg_ptr[i], _InputStream, target);
            }
            
            else if (node_buffer.reg_ptr[i].key > key)
                break;
        }
        
        if (node_buffer.header.is_leaf)
            break;

        index = node_buffer.children_ptr[i];
    }
#endif

    return false;
}

/*  */
static bool
__ReadingTest(void)
{
    FILE * bstream = fopen(TEMPOUT_FILENAME, "rb");
    if (bstream == NULL) {
        fprintf(stderr, "[__ReadingTest] File won't open.\n");
        return false;
    }    

    printf("FILE:\n");
    fseek(bstream, 0, SEEK_SET);
    
    b_node buffer;
    while (fread(& buffer, sizeof(b_node), 1, bstream))
        PrintBNode(& buffer);
    

    // __PrintBTree(bstream);
    fclose(bstream);
    
    fprintf(stderr, "[__ReadingTest] Success.\n");
    return true;
}

/*  */
static bool
__BuildingTest(void)
{
    FILE * input_stream = fopen(TEST_FILENAME, "r");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
        return false;
    }

    FILE * output_stream = fopen(TEMPOUT_FILENAME, "w+b");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output file won't open....\n");
        fclose(input_stream);
        return false;
    }

    if (! BTree_Build(input_stream, output_stream)) {
        fclose(input_stream);
        fclose(output_stream);
        printf("[__BuildingTest] Failure.\n");
        return false;
    }
    
    fclose(input_stream);
    fclose(output_stream);
    printf("[__BuildingTest] Success.\n");
    return true;
}

/*  */
static bool
__SearchTest(int argc, char ** argsv)
{
    if (argc == 2)
        return false;

    key_t x = atoi(argsv[2]);

    FILE * bstream = fopen(TEMPOUT_FILENAME, "rb");
    if (bstream == NULL)
        return false;

    FILE * input_stream = fopen(TEST_FILENAME, "rb");
    if (input_stream == NULL)
    {
        fclose(bstream);
        return false;
    }
    
    DebugPrint("Searching for the key <%u>\n", (unsigned int) x);

    BTreeStream bts = { bstream, 0 };
    
    registry_t target;
    if (BTreeSearch(x, & bts, input_stream, & target)) {
        printf("Registry Found! Key: %d\n", target.key);
        printf("Registry data 1: %ld\nRegistry data 2: %.4s\nRegistry data 3: %.4s\n", 
            (long) target.data_1, target.data_2, target.data_3);
    }
    
    else
        printf("Registry NOT Found!\n");
    

    fclose(bstream);
    fclose(input_stream);
    return false;
}


/*  PROGRAM SPECS.

    * Currently:
        . Accepts one parameter. The string with it has to either start on
        'b' or in 'r';
        . If it is not the case the parser fails and so does the program;
        . The 'b' entry stands for 'building', and then the B-tree will be attempted
        being built, the application returning its succes;
        . The 'r' entry stands for 'reading', and the last B-tree constructed will be
        attempted being read.
*/
int main(int argc, char ** argsv) {
    if (argc < 2 && argc > 3) {
        fprintf(stderr, "[ERROR]: Not yet impl. bro.\n");
        return EXIT_FAILURE;
    }
    
    if (argsv[1][0] == 'b') {
        if (! __BuildingTest())
            return EXIT_FAILURE;
    }
    else if (argsv[1][0] == 'r') {
        if (! __ReadingTest()) 
            return EXIT_FAILURE;
    }
    else if (argsv[1][0] == 's') {
        if (! __SearchTest(argc, argsv))
            return EXIT_FAILURE;
    }
    else 
        EXIT_FAILURE;
    return EXIT_SUCCESS;
}

// Created with the assistance of ChatGPT (OpenAI) - 2022