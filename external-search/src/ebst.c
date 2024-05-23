#include "ebst.h"
#include <assert.h>


// Debug
static void
PrintEBSTNode(const erbt_node * _Node) {
    printf("\t| ");
    printf("\t| left:%3.d   key:%3.d   right:%3.d | ",
        (int) _Node -> left,
        (int) _Node -> reg_ptr.key,
        (int) _Node -> right);

    printf("father: %3.d",
        (int) _Node -> father);

    printf("\n");
}

void printRedBlackTree(EBST_STREAM * _Stream) {
    DebugFuncMark();

    erbt_node tempNode = { 0 };
    uint32_t i = 0;

    while (read_erbtnode(_Stream, i ++, & tempNode)) {
        if (tempNode.color == RED)
            aec_fg_red();
        else
            aec_fg_white();

        printf("%u", i - 1);
        PrintEBSTNode(& tempNode);
        aec_reset();
    }
}

inline static erbt_node 
ERBT_nodeStarter(const registry_pointer * _Entry) {
    erbt_node new_node = {
        .color =    RED,
        .reg_ptr =  * _Entry,
        .father =   ERBT_NULL_INDEX,
        .left =     ERBT_NULL_INDEX,
        .right =    ERBT_NULL_INDEX,
        // .line =     ERBT_NULL_INDEX,
    };

    return new_node;
}


/*  Case of root in inserting into the ERBT. */
inline static bool 
_ERBT_insert_root(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    DebugFuncMark();

    erbt_node new_node = {
        .color = BLACK,
        .reg_ptr = *_Entry,
        .father = -1,
        .left = -1,
        .right = -1,
        // .line = 0,
    };
    _builder -> registries_written ++;
    return write_erbtnode(_builder -> file_stream, 0, &new_node);
}

/*  Inserts a registry in the ERBT data-structure. Returns success. */
static bool ERBT_insert(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    startDebug(); DebugFuncMark();

    // In case of first entry on the binary search tree (root node).
    if (_builder -> registries_written == 0)
        return _ERBT_insert_root(_builder, _Entry);

    /*  The new node to be inserted in the tree. */
    erbt_node newNode = ERBT_nodeStarter(_Entry);

    erbt_node currentNode = { 0 };  // The node over focus on the traversal.
    uint32_t node_index = 0;        // Tracks the index of currentNode.
    
    /*  Sinalizes whether there was an error
        on the inserting process the traversal to halt.
        Failure happens if a node with the same key already
        exists on the structure or a node IO operation fails. */
    bool had_failure = true;

    while (read_erbtnode(_builder -> file_stream, node_index, & currentNode)) {
        /*  As the error sinalizer starts sinalized, in case of failing the
            very first read op., this information is correctly propagated down below.
            Otherwise it is reset for deeper verification. Note that it is always the 
            case if a failure occurred the loop to break. */
        had_failure = false;

        // Check the left first.
        if (newNode.reg_ptr.key < currentNode.reg_ptr.key) {

            /*  Checks whether the current node has a left child. */
            if (currentNode.left == EBST_NULL_INDEX) {
                /*  If it does not, newNode is fit as that.
                    Updates the relationship between them and writes 
                    down newNode as in the last position so far. */

                newNode.father = node_index;
                currentNode.left = _builder -> registries_written;

                // Updating the father.
                if (! write_erbtnode(_builder -> file_stream, node_index, & currentNode))
                    had_failure = true;

                // Adding the new node.
                if (! write_erbtnode(_builder -> file_stream, _builder -> registries_written, & newNode))
                    had_failure = true;
                
                _builder -> registries_written ++;
                break;

            }
            // Navigating left.
            node_index = currentNode.left;
          
        // Check then the right.
        } else if (newNode.reg_ptr.key > currentNode.reg_ptr.key) {
              
            /*  Checks whether the current node has a right child. */
            if (currentNode.right == EBST_NULL_INDEX) {
                /*  If it does not, newNode is fit as that.
                    Updates the relationship between them and writes
                    down newNode as in the last position so far. */
                
                newNode.father = node_index;
                currentNode.right = _builder -> registries_written;

                // Updating the father.
                if (! write_erbtnode(_builder -> file_stream, node_index, & currentNode))
                    had_failure = true;

                // Adding the new node.
                if (! write_erbtnode(_builder -> file_stream, _builder -> registries_written, & newNode))
                    had_failure = true;

                _builder -> registries_written ++;
                break;
            }
            // Navigating right.
            node_index = currentNode.right;

        /*  In case the nodes match keys - neither they're 
            bigger or lower than any another. */
        } else {
            /*  This implies in failure. */
            DebugPrintR("Error: nodes are equal...\n", NULL);
            had_failure = true;
            break;
        }
    }
    endDebug();

    if (had_failure) {
        /*  TODO: Proper debug here. */
        DebugPrintR("Insertion Error: something went wrong...\n", NULL);
        return false;
    }
    return true;
}




/// RedBlackTree exclusive Functions
// Left rotation
static void
rotateLeft(ERBT_Builder * _builder, uint32_t _targetLine) {
    DebugFuncMark();

    erbt_node X, Y, tempNode;

    // Start X & Y
    read_erbtnode(_builder -> file_stream, _targetLine, & X);
    read_erbtnode(_builder -> file_stream, X.right, & Y);

    const ebst_ptr old_x_right = X.right;
    

    /// Operations
    // Y-left sub tree becomes X-right sub tree
    X.right = Y.left;
    Y.father = X.father;

    Y.left = old_x_right;

    // Fix the relation
    X.father = _targetLine;

    // Updates X's right son
    if (X.right != -1) {
        read_erbtnode(_builder->file_stream, X.right, & tempNode);
        tempNode.father = old_x_right;
        write_erbtnode(_builder->file_stream, X.right, & tempNode);
    }

    // Updates X's left son 
    if (X.left != -1) {
        read_erbtnode(_builder->file_stream, X.left, &tempNode);
        tempNode.father = old_x_right;
        write_erbtnode(_builder->file_stream, X.left, &tempNode);
    }

    // Update file
    write_erbtnode(_builder->file_stream, old_x_right, &X);
    write_erbtnode(_builder->file_stream, _targetLine, &Y);
}

// Right rotation
static void
rotateRight(ERBT_Builder * _builder, uint32_t _targetLine) {
    DebugFuncMark();
    // -> gf_index

    // x: gf
    // y: f

    erbt_node X, Y, tempNode;

    // Start X & Y
    read_erbtnode(_builder -> file_stream, _targetLine, & X);
    read_erbtnode(_builder -> file_stream, X.left, & Y);

    const ebst_ptr old_x_left = X.left; // f_index

    // Y-Right sub tree becomes X-Left sub tree
    X.left = Y.right;
    // Updates Y's father (same as X)
    Y.father = X.father;
    // Changes X and Y position
    // TemporaryPosition = old_x_left;

    // Y.line = X.line;
    // X.line = TemporaryPosition;
    
    // Puts X above Y
    Y.right = old_x_left;
    // Fix the relation
    X.father = _targetLine;


    if (X.right != -1) {
        read_erbtnode(_builder -> file_stream, X.right, & tempNode);
        tempNode.father = old_x_left;
        write_erbtnode(_builder -> file_stream, X.right, & tempNode);
    }
    // Updates X's left son 
    if (X.left != -1) {
        read_erbtnode(_builder->file_stream, X.left, &tempNode);
        tempNode.father = old_x_left;
        write_erbtnode(_builder->file_stream, X.left, &tempNode);
    }
    //Update file
    write_erbtnode(_builder->file_stream, old_x_left, &X);
    write_erbtnode(_builder->file_stream, _targetLine, &Y);



}





struct ERBT_Balancer {
    ERBT_Builder * builder;

    erbt_node node, father_node, uncle_node, grandfather_node;
    ebst_ptr node_index, uncle_index;
};


/*  ERBT balance case 1.1 and 2.1: color correction
      BLACK                                  RED
   <-       ->      == Becomes ==>        <-     ->
RED           RED                   BLACK           BLACK
*/
inline static void
_ERBT_Balance_case_change(struct ERBT_Balancer * balancer) {

    startDebug(); DebugFuncMark();
    // CASE 1.1 & 2.1: Uncle also is RED
    balancer -> grandfather_node.color  = RED;
    balancer -> uncle_node.color        = BLACK;
    balancer -> father_node.color       = BLACK;

    write_erbtnode(balancer -> builder -> file_stream, balancer -> father_node.father, & balancer -> grandfather_node);
    write_erbtnode(balancer -> builder -> file_stream, balancer -> uncle_index, & balancer -> uncle_node);
    write_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> father_node);

    // temp
    printRedBlackTree(balancer -> builder -> file_stream);

    // Grandpa becomes the Main Node
    // balancer -> node.line = balancer -> father_node.father;
    balancer -> node_index = balancer -> father_node.father;
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    endDebug();
}

/*  ERBT balance case 1.2: Adapts a node-frame for case 1.3.
*/
inline static void
_ERBT_Balance_case1_2(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();
    // CASE 1.2: Main Node is not at the same side as he's father
    rotateLeft(balancer -> builder, balancer -> node.father);
    
    // Updates the position, bringing the sequence back to Grandpa -> Son -> Father                    
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> father_node);
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    printRedBlackTree(balancer -> builder -> file_stream);
    
    endDebug();
}

/*  CASE 1.3: balancer.node is at the correct side. 
Grandpa                                                         <- Father ->     
        ->                                              Grandpa              Last Node
            Father                  
                    ->
                        Last Node
*/
inline static void
_ERBT_Balance_case1_3(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();

    const int32_t old_x_left = balancer -> grandfather_node.left;

    rotateRight(balancer -> builder, balancer -> father_node.father);

    read_erbtnode(balancer->builder->file_stream, old_x_left, &balancer->father_node);
    read_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    bool father_color = balancer->father_node.color;
    balancer->father_node.color = balancer->grandfather_node.color;
    balancer->grandfather_node.color = father_color;

    write_erbtnode(balancer->builder->file_stream, old_x_left, &balancer->father_node);
    write_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    balancer->node_index = balancer->father_node.father;
    balancer->node = balancer -> grandfather_node;

    // * em caso os outros não importem
    // balancer->node = balancer->grandfather_node;

    printf("-----------------------------------------\n");
    printf("%d, %d, %d\n", balancer->node_index, balancer -> node.father, balancer->father_node.father);

    PrintEBSTNode(&balancer->node);
    PrintEBSTNode(&balancer->father_node);
    PrintEBSTNode(&balancer->grandfather_node);
    printRedBlackTree(balancer->builder -> file_stream);
    printf("-----------------------------------------\n");

    endDebug();
}

/*  ERBT balance case 2.2: Adapts a node-frame for case 2.3.
Grandpa                                  Grandpa
        ->                                       ->
            Father      == Becomes ==>              Last Node
        <-                                                 ->
Last Node                                                     Father
*/
inline static void
_ERBT_Balance_case2_2(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();
    
    rotateRight(balancer -> builder, balancer -> node.father);

    // Updates the node-information, after the rotation, into the balancer - bringing it to main-memory.
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> father_node);
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    endDebug();
}

inline static void
_ERBT_Balance_case_2_3(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();
    

    const int32_t old_x_right = balancer -> grandfather_node.right;

    rotateLeft(balancer -> builder, balancer -> father_node.father);

    read_erbtnode(balancer->builder->file_stream, old_x_right, &balancer->father_node);
    read_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    bool father_color = balancer->father_node.color;
    balancer->father_node.color = balancer->grandfather_node.color;
    balancer->grandfather_node.color = father_color;

    write_erbtnode(balancer->builder->file_stream, old_x_right, &balancer->father_node);
    write_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    balancer->node_index = balancer->father_node.father;
    balancer->node = balancer -> grandfather_node;


/*
    const ebst_ptr old_father_index = balancer -> node.father;

    balancer -> node.father = balancer -> father_node.father;
    write_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    rotateLeft(balancer -> builder, balancer -> father_node.father);

    read_erbtnode(balancer -> builder -> file_stream, balancer -> father_node.father, & balancer -> grandfather_node);
    read_erbtnode(balancer -> builder -> file_stream, old_father_index, & balancer -> father_node);

    bool temp = balancer -> grandfather_node.color;
    balancer -> grandfather_node.color = balancer -> father_node.color;
    balancer -> father_node.color = temp;

    write_erbtnode(balancer -> builder -> file_stream, old_father_index, & balancer -> father_node);
    write_erbtnode(balancer -> builder -> file_stream, balancer -> father_node.father, & balancer -> grandfather_node);

    // balancer -> node.line = balancer -> father_node.father;
    balancer -> node_index = balancer -> father_node.father;

    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);
*/
    endDebug();
}

/*  Balances the External Red-Black Tree. */
static void ERBT_Balance(ERBT_Builder * _builder, uint32_t _NodeIndex) {
    startDebug(); DebugPrint("_XLine: %u\n", (unsigned int) _NodeIndex);

    /*  The balancing process manager. */
    struct ERBT_Balancer balancer = { 0 };
    balancer.builder = _builder;
    balancer.node_index = _NodeIndex;
    balancer.uncle_index = ERBT_NULL_INDEX;
    
    balancer.node = (erbt_node)             { 0 };
    balancer.father_node = (erbt_node)      { 0 };
    balancer.uncle_node = (erbt_node)       { 0 };
    balancer.grandfather_node = (erbt_node) { 0 };


    //balancer.father_node.line = ERBT_NULL_INDEX;
    balancer.father_node.father = ERBT_NULL_INDEX;

    //balancer.grandfather_node.line = ERBT_NULL_INDEX;
    balancer.grandfather_node.father = ERBT_NULL_INDEX;

    //balancer.uncle_node.line = ERBT_NULL_INDEX;

    // Start Main Node 
    read_erbtnode(_builder -> file_stream, balancer.node_index, & balancer.node);

    // Set / update Nodes
    if (balancer.node.father != ERBT_NULL_INDEX) {
        read_erbtnode(_builder -> file_stream, balancer.node.father, & balancer.father_node);

        // Start Gradpa if exists
        if (balancer.father_node.father != ERBT_NULL_INDEX) {
            read_erbtnode(_builder -> file_stream, balancer.father_node.father, & balancer.grandfather_node);
        }
    }
    
    DebugPrintY("Before while...\n", NULL);

    // If: the balancer.node is root or balancer.node is Black or Father is Black, the balancing ends
    /* (...) */
    while ((balancer.node_index != 0) && (balancer.node.color == RED) && (balancer.father_node.color == RED))
    {
        DebugPrintY("Inside the while loop...\n\t", NULL);

        // In case the father node is at left,
        if (balancer.node.father == balancer.grandfather_node.left) {
            printf("\t"); DebugPrintY("father is left...\n", NULL);

            // Set main uncle
            if ((balancer.father_node.father != ERBT_NULL_INDEX) && (balancer.grandfather_node.right != ERBT_NULL_INDEX)) {
                read_erbtnode(_builder -> file_stream, balancer.grandfather_node.right, & balancer.uncle_node);
                balancer.uncle_index = balancer.grandfather_node.right;

            }
            else {
                balancer.uncle_node = (erbt_node) { 0 };
                // balancer.uncle_node.line = ERBT_NULL_INDEX;
                balancer.uncle_node.father = ERBT_NULL_INDEX;

                balancer.uncle_index = ERBT_NULL_INDEX;
            }

            // CASE 1.1 Uncle is RED
            if ((balancer.uncle_index != ERBT_NULL_INDEX) && (balancer.uncle_node.color == RED))
                _ERBT_Balance_case_change(& balancer);

            else {
                // CASE 1.2: Main Node is not at the same side as he's father
                if (balancer.node_index == balancer.father_node.right)
                    _ERBT_Balance_case1_2(& balancer);
                
                // CASE 1.3: Main Node is at the correct side
                _ERBT_Balance_case1_3(& balancer);
            }
        
        // In case the father node is at the right,
        } else {
            DebugPrintY("father is right...\n", NULL);

            // Set balancer.uncle_node
            if ((balancer.father_node.father != ERBT_NULL_INDEX) && (balancer.grandfather_node.left != ERBT_NULL_INDEX)) {
                read_erbtnode(_builder -> file_stream, balancer.grandfather_node.left, & balancer.uncle_node);
                balancer.uncle_index = balancer.grandfather_node.left;

            }
            else {
                balancer.uncle_node = (erbt_node) { 0 };
                // balancer.uncle_node.line = ERBT_NULL_INDEX;
                balancer.uncle_node.father = ERBT_NULL_INDEX;

                balancer.uncle_index = ERBT_NULL_INDEX;
            }
            // CASE 2.1 Uncle is RED
            if ((balancer.uncle_index != ERBT_NULL_INDEX) && (balancer.uncle_node.color == RED)) {
                _ERBT_Balance_case_change(& balancer);

            } else {
                // CASE 2.2: Main Node is not at the same side as he's father
                if (balancer.node_index == balancer.father_node.left)
                    _ERBT_Balance_case2_2(& balancer);

                // CASE 2.3: Main Node is at the correct side
                _ERBT_Balance_case_2_3(& balancer);

            }
        }

        // Set/update Nodes
        if (balancer.node.father != ERBT_NULL_INDEX) {
            read_erbtnode(_builder -> file_stream, balancer.node.father, & balancer.father_node);

            if (balancer.father_node.father != ERBT_NULL_INDEX) {
                read_erbtnode(_builder -> file_stream, balancer.father_node.father, & balancer.grandfather_node);
            }
            else {
                balancer.father_node.father = ERBT_NULL_INDEX;
                balancer.grandfather_node = (erbt_node) { 0 };
            }
        }
        else {
            balancer.father_node = (erbt_node) { 0 };
            balancer.grandfather_node = (erbt_node) { 0 };

            balancer.father_node.father = ERBT_NULL_INDEX;
        }
    }

    // Make sure that the root is BLACK
    erbt_node root;
    read_erbtnode(_builder -> file_stream, 0, & root);
    root.color = BLACK;
    write_erbtnode(_builder -> file_stream, 0, & root);

    endDebug();
}


/*  Generates the external data-structure of the red-black tree. 
    Functionally reads page per page from the input stream and inserts, 
    balacing afterwards the tree, registry per registry in it. */
bool ERBT_Build(REG_STREAM * _InputStream, EBST_STREAM * _OutputStream) {
    DebugFuncMark();

    // Where pages from _InputStream are lied down.
    regpage_t page_buffer = { 0 };      
    
    /*  The registry data that will be passed to the
        the data-structure assembling. */
    registry_pointer reg_ptr = { 0 };   

    // The handler in the data-structure assembling process.
    ERBT_Builder builder = { 0 };       


    if (! makeFrame(& builder.frame, sizeof(erbt_node)))
        return false;
    /*  If it is was not possible to make the frame the whole building process fails.
        Otherwise what it is left to initialize on the builder is properly done. */
    builder.file_stream = _OutputStream;
    builder.registries_written = 0;

    // Counters that tracks the reading progress of the input-stream.
    uint32_t regs_read = 0, currentPage = 0;

    // Sinalizes whether there was an error in inserting a registry data into the tree.
    bool insert_failure = false;


    while ((! insert_failure) && ((regs_read = read_regpage(_InputStream, currentPage ++, & page_buffer)) > 0))
    {
        DebugPrintR("Loop, page: #%u\n", (unsigned int) currentPage);

        for (uint32_t i = 0; i < regs_read; i ++) {
            reg_ptr.key = page_buffer.reg[i].key;

            printf("reg <%u, %d>\n",
                reg_ptr.key, reg_ptr.original_pos);
            fflush(stdout);

            if (! ERBT_insert(& builder, & reg_ptr)) {
                insert_failure = false;
                break;
            }
            ERBT_Balance(& builder, builder.registries_written - 1);

            printRedBlackTree(builder.file_stream);

            reg_ptr.original_pos ++;
        }

        if (currentPage > 3) {
            break;
        }


    }
    // freeFrame(.frame);
    return ! insert_failure;
}













struct mrt_stack_item { ebst_ptr left, right; };

typedef struct mrt_stack_node_t {
    struct mrt_stack_item item;
    struct mrt_stack_node_t * next;
} mrt_stack_node;

typedef struct {
    uint32_t size;
    mrt_stack_node * head;
} MRT_Stack;


static mrt_stack_node * new_mrtStackNode(const struct mrt_stack_item * _Item)
{
    mrt_stack_node * sn = (mrt_stack_node *) malloc(sizeof(mrt_stack_node));
    if (sn == NULL)
        return NULL;


    sn -> item = * _Item;
    sn -> next = NULL;
    return sn;
}

#define del_mrtStackNode(_node_ptr)     free(_node_ptr)


static bool mrtStackPush(MRT_Stack * _Stack, const struct mrt_stack_item * _Item)
{
    mrt_stack_node * new_node = new_mrtStackNode(_Item);
    if (new_node == NULL)
        return false;

    mrt_stack_node * head_next = _Stack -> head;
    _Stack -> head = new_node;
    _Stack -> head -> next = head_next;
    _Stack -> size ++;

    return true;
}

static bool mrtStackPop(MRT_Stack * _Stack, struct mrt_stack_item * _ReturnItem)
{
    if (_Stack->head == NULL)
        return false;


    mrt_stack_node * node_buffer = _Stack->head;

    _Stack->head = _Stack->head->next;
    _Stack->size--;

    *_ReturnItem = node_buffer->item;
    del_mrtStackNode(node_buffer);

    return true;
}

finline static bool
write_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, const ebst_node * _WriteNode)
{
    fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fwrite(_WriteNode, sizeof(ebst_node), 1, _Stream) > 0;
}

finline static bool
read_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, ebst_node * _Node)
{
    fseek(_Stream, sizeof(ebst_node) * _NodeIndex, SEEK_SET);
    return fread(_Node, sizeof(ebst_node), 1, _Stream) > 0;
}

bool EBST_MRT_Build(REG_STREAM * _InputStream, EBST_STREAM * _OutputStream, frame_t * _Frame, bool ascending, uint64_t _RegistriesQtt)
{
    // * for instance.
    if (! ascending)
        return false;

    // The pointers in the subfile division of the input-stream.
    long long left = 0, right = ((long long) _RegistriesQtt) - 1, middle = 0;

    // * stack initialization
    struct mrt_stack_item split_buffer = { left, right };

    MRT_Stack split_stack = { 0 };
    mrtStackPush(& split_stack, & split_buffer);

    ebst_node buffer_node = { 0 };

    uint32_t iterator = 0;
    uint32_t page_pointer = 0;
    
    while (mrtStackPop(& split_stack, & split_buffer))
    {
        left = split_buffer.left; right = split_buffer.right;
        middle = midpoint(left, right);

        buffer_node.left = EBST_NULL_INDEX;
        buffer_node.right = EBST_NULL_INDEX;

        if (left <= (middle - 1))
        {
            split_buffer.right = middle - 1;
            mrtStackPush(&split_stack, & split_buffer);

            buffer_node.left = iterator + 1 + rightSubfileSize(left, right);
        }

        if ((middle + 1) <= right)
        {
            split_buffer.left = middle + 1;
            split_buffer.right = right;

            mrtStackPush(& split_stack, & split_buffer);

            buffer_node.right = iterator + 1;
        }

        // seekReadRegistry(middle, & buffer_node.root_item)
        
        retrieve_regpage(_InputStream, _Frame, middle / ITENS_PER_PAGE, & page_pointer);
        buffer_node.reg_ptr.original_pos = middle;
        buffer_node.reg_ptr.key = ((regpage_t *) _Frame->pages)[page_pointer].reg[middle % ITENS_PER_PAGE].key;

        write_ebstnode(_OutputStream, iterator, & buffer_node);


        if (iterator > 2) {
            break;
        }

        iterator ++;
    }

    return true;
}

bool EBST_Search(EBST_STREAM * _Stream, REG_STREAM * _InputStream, const key_t _Key, registry_t * _Target)
{

    int pos = 0;
    ebst_node node_buffer;

    while (pos >= 0)
    { 
        read_ebstnode(_Stream, pos, & node_buffer);
        printf(">>> ebstnode %d: [(%d, %d), <%u, %d>]\n", pos,
            node_buffer.left, node_buffer.right, 
            node_buffer.reg_ptr.original_pos, node_buffer.reg_ptr.key);


        if (_Key == node_buffer.reg_ptr.key)
        {
            printf(">>> eq\n");
            return search_registry(_InputStream, & node_buffer.reg_ptr, _Target);
        }
        else if (_Key < node_buffer.reg_ptr.key)
        {
            // end of search
            if (node_buffer.left == EBST_NULL_INDEX)
                break;

            pos = node_buffer.left;
        }
        else {
            if (node_buffer.right == EBST_NULL_INDEX)
                break;

            pos = node_buffer.right;
        }


    }
    return false;
}
