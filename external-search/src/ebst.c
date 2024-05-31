/*  <src/ebst.c>

*/


#include "ebst.h"


/*  Writes the header of the ERBT stream. Returns success. */
inline static bool
ERBT_writeHeader(ERBT_STREAM * _Stream, const ERBT_Header * _Header)
{
    fseek(_Stream, erbt_header_pos(), SEEK_SET);
    return fwrite(_Header, sizeof(ERBT_Header), 1, _Stream) > 0;
}

/*  Reads the header of the ERBT stream. Returns success. */
inline static bool
ERBT_readHeader(ERBT_STREAM * _Stream, ERBT_Header * _ReturnHeader)
{
    fseek(_Stream, erbt_header_pos(), SEEK_SET);
    return fread(_ReturnHeader, sizeof(ERBT_Header), 1, _Stream) > 0;
}


/*  If set, after ERBT_Build, the tree will be written out
    into a temporary text file. It follows the convention
    used in tests. As an effect, once a construction result
    could be properly verified being correct, the result can
    be propagated as a test case. */
#define RECORD_ERBT false


// The debugging set-up for the ERBT.
#if IMPL_LOGGING

#if RECORD_ERBT
    #define RBT_PRINTING_FILENAME   "last-rbt.txt"
    FILE * RBT_PRINTING_STREAM = NULL;
#endif // RECORD_ERBT


static void fPrintERBTNode(const erbt_node * _Node, FILE * _Stream) {
    fprintf(_Stream, "\t| (left:%3d, right:%3d), key:%3d | ",
        (int) _Node -> left,
        (int) _Node -> right,
        (int) _Node -> reg_ptr.key);

    fprintf(_Stream, "father: %3.d, ",
        (int) _Node -> father);

    if (_Node->color == RED)
        fprintf(_Stream, "RED");
    else
        fprintf(_Stream, "BLACK");

    fprintf(_Stream, "\n");
}

#define PrintERBTNode(_Node)    fPrintERBTNode(_Node, debug_stream)


void printRedBlackTree(ERBT_STREAM * _Stream) {
    // tracking how many times this function has been called.
    static uint32_t printing_times = 0;
    printing_times ++;

    DebugPrintf("#%u printing:\n", (unsigned int) printing_times);
    // fprintf(RBT_PRINTING_STREAM, "#%u:\n", (unsigned int) printing_times);

    erbt_node buffer_node = { 0 };
    uint32_t i = 0;

    while (read_erbtnode(_Stream, i ++, & buffer_node)) {
    
#if STDERR_DEBUG_LOGGING
        if (buffer_node.color == RED)
            aec_fg_red();
        else
            aec_fg_white();
#endif // STDERR_DEBUG_LOGGING

        printDebugSpacing();
        fprintf(debug_stream, "#%u", (unsigned int) (i - 1));
        fPrintERBTNode(& buffer_node, debug_stream);

#if STDERR_DEBUG_LOGGING
        aec_reset();
#endif // STDERR_DEBUG_LOGGING
    }
}


#if RECORD_ERBT
/*  Writes the ERBT into a file on the current convention, given the ERBT stream. */
static inline void __RBT_WRITE(ERBT_STREAM * _Stream, size_t _NodesQtt)
{
    ERBT_Header header = { 0 };
    ERBT_readHeader(_Stream, & header);

    fprintf(RBT_PRINTING_STREAM, "%u\n", (unsigned int) header.root_ptr);

    erbt_node buffer_node = { 0 };
    size_t iterator = 0;
    while (read_erbtnode(_Stream, iterator ++, & buffer_node)) {
        fprintf(RBT_PRINTING_STREAM, "%d %d %d %d ",
            (int) buffer_node.reg_ptr.key,
            (int) buffer_node.left,
            (int) buffer_node.right,
            (int) buffer_node.father);

        fprintf(RBT_PRINTING_STREAM, 
            buffer_node.color == RED ? "RED" : "BLACK");
        fputc('\n', RBT_PRINTING_STREAM);
    }

    if ((iterator - 1) != _NodesQtt) {
        fprintf(RBT_PRINTING_STREAM, "\niterator: %u, _NodesQtt: %u; error\n",
            (unsigned int) iterator, (unsigned int) _NodesQtt);
    }
}
#endif // RECORD_ERBT

#endif // IMPL_LOGGING


/*  Initializes a standard node for insertion. 
    It is such that it has its pointers nulled (by ERBT_NULL_INDEX)
    and is red. */
inline static erbt_node 
ERBT_nodeStarter(const registry_pointer * _Entry) {
    erbt_node new_node = {
        .color =    RED,
        .reg_ptr =  * _Entry,
        .father =   ERBT_NULL_INDEX,
        .left =     ERBT_NULL_INDEX,
        .right =    ERBT_NULL_INDEX,
    };
    return new_node;
}

/*  Case of root inserting in inserting into the ERBT. */
inline static bool 
_ERBT_insert_root(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    erbt_node new_node = ERBT_nodeStarter(_Entry);
    new_node.color = BLACK;
    
    _builder -> registries_written ++;
    return write_erbtnode(_builder -> file_stream, _builder -> header.root_ptr, & new_node);
}

/*  Inserts a registry in the ERBT data-structure. Returns success. */
bool ERBT_insert(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    #if IMPL_LOGGING
        raiseDebug(); 
    #endif

    // In case of first entry on the binary search tree (root node).
    if (_builder -> registries_written == 0) {
#if IMPL_LOGGING
        bool insert_root_r = _ERBT_insert_root(_builder, _Entry);
        fallDebug();
        return insert_root_r;
#else
        return _ERBT_insert_root(_builder, _Entry);
#endif
    }

    /*  The new node to be inserted in the tree. */
    erbt_node newNode = ERBT_nodeStarter(_Entry);

    erbt_node currentNode = { 0 };                      // The node over focus on the traversal.
    uint32_t node_index = _builder -> header.root_ptr;  // Tracks the index of currentNode.
    
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

            #if IMPL_LOGGING
                        DebugPrintR("Error: nodes are equal...\n", NULL);
            #endif
            
            had_failure = true;
            break;
        }
    }

#if IMPL_LOGGING
    if (had_failure) {
        /*  TODO: Proper debug here. */
        DebugPrintR("Insertion Error: something went wrong...\n", NULL);
    }
    fallDebug();
#endif
    return ! had_failure;
}


/*  Structures a right rotation around the passed pivot in the tree.
    Manages to update only the branching pointers in the data-structure stream. 
 
    Define X as pivot node and Y as its original left child.

    Invariants of use:
        | _pivotIndex != EBST_NULL_INDEX;
        | X has to have a left child.

    The six following operations are done in order to guaranteeing the
    after-rotation properties:
    
    1. X's left subtree becomes Y's right one.
    2. X's father becomes Y's one instead.
    3. Y will have X as its right child.
    4. In concordance to (2), Y takes X's child place on X's father.
    5. In concordance to (3), X adopts Y as its father.
    6. In concordance to (1), the right subtree of Y will have X as its root. */
static void
rotateRight(ERBT_Builder * _builder, const ebst_ptr _pivotIndex) 
{
#if IMPL_LOGGING
    raiseDebug();
    DebugPrintf("pivot: %d\n", _pivotIndex);
#endif 

    if (_pivotIndex == EBST_NULL_INDEX)
    {
        // * error: quitting in order to prevent program confusion
        #if IMPL_LOGGING
                DebugPrintR("error\n", NULL);
        #endif

        return;
    }

    /*  X: pivot-node; Y: its left child. */
    erbt_node X = { 0 }, Y = { 0 };
    read_erbtnode(_builder -> file_stream, _pivotIndex, & X);
    read_erbtnode(_builder -> file_stream, X.left, & Y);

    /*  Tracks Y index - as X.left will be updated. */
    const ebst_ptr old_x_left = X.left;


    // 1. Y's right subtree becomes X's left one.
    X.left = Y.right;
 
    // 2. Y's father becomes the same as X's original one.
    Y.father = X.father;

    // 3. Y adopts X as its right child.
    Y.right = _pivotIndex;

    /*  4. Propagating the X-Y substitution as child information 
        up to X's father. */
    if (X.father != ERBT_NULL_INDEX) {
        erbt_node x_father;
        read_erbtnode(_builder -> file_stream, X.father, & x_father);

        // X being originally the right-child, resets its pointer to Y's index;
        if (x_father.right == _pivotIndex)
            x_father.right = old_x_left;
        
        // does so equally X being the left one.
        else
            x_father.left = old_x_left;
        
        write_erbtnode(_builder -> file_stream, X.father, & x_father);
    }
    else {
        // Then _pivotIndex == _builder -> header.root_ptr.
        _builder -> header.root_ptr = old_x_left;
        ERBT_writeHeader(_builder -> file_stream, & _builder -> header);
    }

    // 5. In concordance with (.3), X recognizes Y as its father.
    X.father = old_x_left;

    // 6. Tells the before Y's right subtree that X become its root.
    if (X.left != ERBT_NULL_INDEX) {
        erbt_node y_right_subtree_root;
        read_erbtnode(_builder -> file_stream, X.left, & y_right_subtree_root);
        y_right_subtree_root.father = _pivotIndex;
        write_erbtnode(_builder -> file_stream, X.left, & y_right_subtree_root);
    }
    
    // Updating target and its left child in their respective positions. (Position is preserved)
    write_erbtnode(_builder -> file_stream, old_x_left, & Y);
    write_erbtnode(_builder -> file_stream, _pivotIndex, & X);

#if IMPL_LOGGING
    printRedBlackTree(_builder -> file_stream);
    fallDebug();
#endif
}


/*  Structures a left rotation aroung the passe pivot in the tree.
    (...)

    TODO: docs.
    TODO: rotates -> bool on return.
*/
static void
rotateLeft(ERBT_Builder * _builder, const ebst_ptr _pivotIndex) 
{
#if IMPL_LOGGING
    raiseDebug();
    DebugPrintf("pivot: %d\n", _pivotIndex);
#endif

    /*  X: pivot-node; Y: its right child. */
    erbt_node X, Y;
    read_erbtnode(_builder -> file_stream, _pivotIndex, & X);
    read_erbtnode(_builder -> file_stream, X.right, & Y);

    /*  Tracks Y index - as X.right will be updated. */
    const ebst_ptr old_x_right = X.right;


    // 1. Y's left subtree becomes X's right one.
    X.right = Y.left;

    // 2. Y's father becomes the same as X's original one.
    Y.father = X.father;

    // 3. Y adopts X as its left child.
    Y.left = _pivotIndex;

    /*  4. Propagating the X-Y substitution as child information
        up to X's father. */
    if (X.father != ERBT_NULL_INDEX) {
        erbt_node x_father;
        read_erbtnode(_builder -> file_stream, X.father, & x_father);

        // X being originally the right-child, resets its pointer to Y's index;
        if (x_father.right == _pivotIndex)
            x_father.right = old_x_right;

        // does so equally X being the left one.
        else
            x_father.left = old_x_right;

        write_erbtnode(_builder -> file_stream, X.father, & x_father);
    }
    else {
        // Then _pivotIndex == _builder -> header.root_ptr.
        _builder -> header.root_ptr = old_x_right;
        ERBT_writeHeader(_builder -> file_stream, & _builder -> header);
    }
    

    // 5. In concordance with (.3), X recognizes Y as its father.
    X.father = old_x_right;

    // 6. Tells the before Y's left subtree that X become its root.
    if (X.right != ERBT_NULL_INDEX) {
        erbt_node y_left_subtree_root;
        read_erbtnode(_builder -> file_stream, X.right, & y_left_subtree_root);
        y_left_subtree_root.father = _pivotIndex;
        write_erbtnode(_builder -> file_stream, X.right, & y_left_subtree_root);
    }
    
    // Updating target and its left child in their respective positions. (Position is preserved)
    write_erbtnode(_builder -> file_stream, old_x_right, & Y);
    write_erbtnode(_builder -> file_stream, _pivotIndex, & X);

#if IMPL_LOGGING
    printRedBlackTree(_builder -> file_stream);
    fallDebug();
#endif
}


/*  Holds on the information in order to auxiliate
    in the balancing process of the ERBT. */
struct ERBT_Balancer {
    ERBT_Builder * builder;

    erbt_node node, father_node, uncle_node, grandfather_node;
    ebst_ptr node_index, uncle_index;
};

/*  Initializes properly the balancer. */
finline static struct ERBT_Balancer
_initializeBalancer(ERBT_Builder * _builder, ebst_ptr _NodeIndex) {
    struct ERBT_Balancer balancer = { 0 };
    balancer.builder = _builder;
    balancer.node_index = _NodeIndex;
    balancer.uncle_index = ERBT_NULL_INDEX;
    
    balancer.node = (erbt_node)             { 0 };
    balancer.father_node = (erbt_node)      { 0 };
    balancer.uncle_node = (erbt_node)       { 0 };
    balancer.grandfather_node = (erbt_node) { 0 };
    balancer.father_node.father = ERBT_NULL_INDEX;
    balancer.grandfather_node.father = ERBT_NULL_INDEX;

    return balancer;
}

/*  Updates the information erbt nodes <father> and <grandfather> tracked by the balancer,
    according to the current node and its index. */
inline static bool
updateBalacer(struct ERBT_Balancer * _Balancer)
{
#if IMPL_LOGGING
    raiseDebug();
#endif

    if (_Balancer -> node.father != ERBT_NULL_INDEX) {
#if IMPL_LOGGING
        DebugPrintf("Reading father <%u>\n", (unsigned int) _Balancer -> node.father);
#endif
        if (! read_erbtnode(_Balancer->builder->file_stream, _Balancer->node.father, &_Balancer->father_node))
        {
#if IMPL_LOGGING
            fallDebug();
#endif

            return false;
        }

        if (_Balancer -> father_node.father != ERBT_NULL_INDEX) {
#if IMPL_LOGGING
            DebugPrintf("Reading grandfather <%u>\n", (unsigned int) _Balancer -> father_node.father);
#endif

            if (!read_erbtnode(_Balancer->builder->file_stream, _Balancer->father_node.father, &_Balancer->grandfather_node))
            {
#if IMPL_LOGGING
                fallDebug();
#endif
                return false;
            }
        }
        
        else {
#if IMPL_LOGGING
            DebugPrint("Not reading grandfather <%u>\n");
#endif

            _Balancer -> grandfather_node = (erbt_node){ 0 };
        }
    }
    else {
#if IMPL_LOGGING
        DebugPrint("Not reading father <%u>\n");
#endif

        _Balancer -> father_node = (erbt_node){ 0 };
        _Balancer -> grandfather_node = (erbt_node){ 0 };

        _Balancer -> father_node.father = ERBT_NULL_INDEX;
    }

#if IMPL_LOGGING
    fallDebug();
#endif

    return true;
}


/*  
    TODO: better docs.

ERBT balance case 1.1 and 2.1: color correction
      BLACK                                  RED
   <-       ->      == Becomes ==>        <-     ->
RED           RED                   BLACK           BLACK
*/
inline static void
_ERBT_Balance_case_change(struct ERBT_Balancer * balancer) 
{
#if IMPL_LOGGING
    raiseDebug();
#endif
    
    // CASE 1.1 & 2.1: Uncle also is RED
    balancer -> grandfather_node.color = RED;
    balancer -> uncle_node.color = BLACK;
    balancer -> father_node.color = BLACK;

    write_erbtnode(balancer -> builder -> file_stream, balancer -> father_node.father, & balancer -> grandfather_node);
    write_erbtnode(balancer -> builder -> file_stream, balancer -> uncle_index, & balancer -> uncle_node);
    write_erbtnode(balancer-> builder -> file_stream, balancer -> node.father, & balancer -> father_node);

    // Grandpa becomes the Main Node
    // balancer -> node.line = balancer -> father_node.father;
    balancer -> node_index = balancer -> father_node.father;
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

#if IMPL_LOGGING
    fallDebug();
#endif
}

/*  ERBT balance case 1.2: Adapts a node-frame for case 1.3.
    TODO: better docs. */
inline static void
_ERBT_Balance_case1_2(struct ERBT_Balancer * balancer) {
#if IMPL_LOGGING
    raiseDebug();
#endif
    // CASE 1.2: Main Node is not at the same side as he's father
    rotateLeft(balancer -> builder, balancer -> node.father);

    // Updates the position, bringing the sequence back to Grandpa -> Son -> Father  
    /*  For passing to case 1.3, the node will now be that that was originally the father...

    */
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> node);
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> father_node);

    balancer -> node_index = balancer -> node.father; // * not necessary, but this implies in concordance...
    read_erbtnode(balancer -> builder -> file_stream, balancer -> father_node.father, & balancer -> grandfather_node);

#if IMPL_LOGGING
    DebugPrintf("After case 1.2:\n", NULL);
    printRedBlackTree(balancer -> builder -> file_stream);
    fallDebug();
#endif
}

/*  TODO: better docs. CASE 1.3: balancer.node is at the correct side.
Grandpa                                                         <- Father ->
        ->                                              Grandpa              Last Node
            Father
                    ->
                        Last Node
*/
inline static void
_ERBT_Balance_case1_3(struct ERBT_Balancer * balancer) {
#if IMPL_LOGGING
    raiseDebug();
    DebugPrintf("Rotate right for grandfather: %u\n", (unsigned int) balancer->father_node.father);
#endif

    // * Note: the positions doesn't change with the rotation, only the pointer in the stream.
    rotateRight(balancer -> builder, balancer -> father_node.father);

    /*  At this point, the color between the X and Y (on the rotation context)
        have to be switched.

        X corresponds to the grandfather, at the step that Y is the father. */
#if IMPL_LOGGING
    DebugPrintf("Various transformations... %d %d %d\n", 
        balancer -> node_index, balancer -> node.father, balancer -> father_node.father);
#endif

    // Bringing them back to the balancer - main memory.
    // const ebst_ptr father_index = balancer -> node.father;
    const ebst_ptr grandfather_index = balancer -> father_node.father;
    read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, &balancer -> father_node);
    read_erbtnode(balancer -> builder -> file_stream, grandfather_index, &balancer -> grandfather_node);

    bool father_color = balancer->father_node.color;
    balancer->father_node.color = balancer->grandfather_node.color;
    balancer->grandfather_node.color = father_color;

    // Writing them back.
    write_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->father_node);
    write_erbtnode(balancer->builder->file_stream, grandfather_index, &balancer->grandfather_node);

    // The balancing afterwards continues on current-node's father. So,
    read_erbtnode(balancer->builder->file_stream, grandfather_index, & balancer -> node);
    balancer -> node_index = grandfather_index;
    // balancer->father_ndoe = erbt_node{ 0 };

}

/*  TODO: better docs. ERBT balance case 2.2: Adapts a node-frame for case 2.3. */
inline static void
_ERBT_Balance_case2_2(struct ERBT_Balancer * balancer) {
#if IMPL_LOGGING
    raiseDebug();
#endif

    rotateRight(balancer->builder, balancer->node.father);

    // Updates the node-information, after the rotation, into the balancer - bringing it to main-memory.
    // read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> father_node);
    //read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->node);
    read_erbtnode(balancer->builder->file_stream, balancer->node_index, &balancer->father_node);

    balancer->node_index = balancer->node.father; // * not necessary, but this implies in concordance...
    read_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

#if IMPL_LOGGING
    DebugPrintf("After case 1.2:\n", NULL);
    printRedBlackTree(balancer->builder->file_stream);
    fallDebug();
#endif
}

/*  ... */
inline static void
_ERBT_Balance_case_2_3(struct ERBT_Balancer * balancer) {
#if IMPL_LOGGING
    raiseDebug();
    DebugPrintf("Rotate left for grandfather: %u\n", (unsigned int) balancer->father_node.father);
#endif

    rotateLeft(balancer->builder, balancer->father_node.father);

    // Bringing them back to the balancer - main memory.
    const ebst_ptr grandfather_index = balancer->father_node.father;
    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->father_node);
    read_erbtnode(balancer->builder->file_stream, grandfather_index, &balancer->grandfather_node);

    bool father_color = balancer->father_node.color;
    balancer->father_node.color = balancer->grandfather_node.color;
    balancer->grandfather_node.color = father_color;

    // Writing them back.
    write_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->father_node);
    write_erbtnode(balancer->builder->file_stream, grandfather_index, &balancer->grandfather_node);

    // The balancing afterwards continues on current-node's father. So,
    read_erbtnode(balancer->builder->file_stream, grandfather_index, &balancer->node);
    balancer->node_index = grandfather_index;

#if IMPL_LOGGING
    fallDebug();
#endif
}


/*  Do the role of setting the root black, at the end of each balancing. */
static inline bool
ERBT_set_root_black(ERBT_Builder * _builder)
{
    erbt_node buffer = { 0 };
    if (! read_erbtnode(_builder -> file_stream, _builder -> header.root_ptr, & buffer))
        return false;

    if (buffer.color == RED) {
        buffer.color = BLACK;
        if (! write_erbtnode(_builder -> file_stream, _builder -> header.root_ptr, & buffer))
            return false;
    }
    return true;
}

/*  Balances the External Red-Black Tree. */
void ERBT_Balance(ERBT_Builder * _builder, ebst_ptr _NodeIndex) {
#if IMPL_LOGGING
    raiseDebug();
    DebugPrintf("_XLine: %u\n", (unsigned int) _NodeIndex);
#endif 

    /*  The balancing process manager. */
    struct ERBT_Balancer balancer = _initializeBalancer(_builder, _NodeIndex);
    
    read_erbtnode(balancer.builder->file_stream, balancer.node_index, &balancer.node);
    updateBalacer(& balancer);

    // If: the balancer.node is root or balancer.node is Black or Father is Black, the balancing ends
    /* (...) */
    while ((balancer.node_index != 0) && (balancer.father_node.color == RED) && (balancer.father_node.father != -1)) {
        
#if IMPL_LOGGING
        DebugPrintY("Inside the while loop...\n\t", NULL);
#endif

        // In case the father node is at left,
        if (balancer.node.father == balancer.grandfather_node.left) {
#if IMPL_LOGGING
            printf("\t"); DebugPrintY("father is left...\n", NULL);
#endif

            // Set main uncle
            if (balancer.grandfather_node.right != ERBT_NULL_INDEX) {
                read_erbtnode(_builder->file_stream, balancer.grandfather_node.right, &balancer.uncle_node);
                balancer.uncle_index = balancer.grandfather_node.right;

            }
            else {
                balancer.uncle_node = (erbt_node){ 0 };
                // balancer.uncle_node.line = ERBT_NULL_INDEX;
                balancer.uncle_node.father = ERBT_NULL_INDEX;

                balancer.uncle_index = ERBT_NULL_INDEX;
            }

            // CASE 1.1 Uncle is RED
            if ((balancer.uncle_index != ERBT_NULL_INDEX) && (balancer.uncle_node.color == RED))
                _ERBT_Balance_case_change(&balancer);

            else {
                // CASE 1.2: Main Node is not at the same side as he's father
                if (balancer.node_index == balancer.father_node.right)
                    _ERBT_Balance_case1_2(&balancer);

                // CASE 1.3: Main Node is at the correct side
                _ERBT_Balance_case1_3(& balancer);
            }

            // In case the father node is at the right,
        }
        else {
#if IMPL_LOGGING
            DebugPrintY("father is right...\n", NULL);
#endif

            // Set balancer.uncle_node
            if ((balancer.father_node.father != ERBT_NULL_INDEX) && (balancer.grandfather_node.left != ERBT_NULL_INDEX)) {
                read_erbtnode(_builder->file_stream, balancer.grandfather_node.left, &balancer.uncle_node);
                balancer.uncle_index = balancer.grandfather_node.left;

            }
            else {
                balancer.uncle_node = (erbt_node){ 0 };
                // balancer.uncle_node.line = ERBT_NULL_INDEX;
                balancer.uncle_node.father = ERBT_NULL_INDEX;

                balancer.uncle_index = ERBT_NULL_INDEX;
            }
            // CASE 2.1 Uncle is RED
            if ((balancer.uncle_index != ERBT_NULL_INDEX) && (balancer.uncle_node.color == RED)) {
                _ERBT_Balance_case_change(& balancer);

            }
            else {
                // CASE 2.2: Main Node is not at the same side as he's father
                if (balancer.node_index == balancer.father_node.left)
                    _ERBT_Balance_case2_2(&balancer);

                // CASE 2.3: Main Node is at the correct side
                _ERBT_Balance_case_2_3(&balancer);

            }
        }

        // Set/update Nodes
        if (balancer.node.father != ERBT_NULL_INDEX) {
            read_erbtnode(_builder->file_stream, balancer.node.father, &balancer.father_node);

            if (balancer.father_node.father != ERBT_NULL_INDEX) {
                read_erbtnode(_builder->file_stream, balancer.father_node.father, &balancer.grandfather_node);
            }
            else {
                balancer.father_node.father = ERBT_NULL_INDEX;
                balancer.grandfather_node = (erbt_node){ 0 };
            }
        }
        else {
            balancer.father_node = (erbt_node){ 0 };
            balancer.grandfather_node = (erbt_node){ 0 };

            balancer.father_node.father = ERBT_NULL_INDEX;
        }

        updateBalacer(& balancer);
    }

    if (! ERBT_set_root_black(_builder)) {
        // TODO: Futurely, the balancing is better verified.
    }
        
#if IMPL_LOGGING
    fallDebug();
#endif
}


/*  Generates the external data-structure of the red-black tree. 
    Functionally reads page per page from the input stream and inserts, 
    balacing afterwards the tree, registry per registry in it. */
bool ERBT_Build(REG_STREAM * _InputStream, EBST_STREAM * _OutputStream) {
#if IMPL_LOGGING
    raiseDebug();

#if RECORD_ERBT
    RBT_PRINTING_STREAM = fopen(RBT_PRINTING_FILENAME, "w");
    if (RBT_PRINTING_STREAM == NULL)
        return false;
#endif

#endif

    // Where pages from _InputStream are lied down.
    regpage_t page_buffer = { 0 };      
    
    /*  The registry data that will be passed to the
        the data-structure assembling. */
    registry_pointer reg_ptr = { 0 };   

    // The handler in the data-structure assembling process.
    ERBT_Builder builder = { 0 };       

    if (! frame_make(& builder.frame, PAGES_PER_FRAME, sizeof(erbt_node), ERBT_PAGE))
        return false;
    /*  If it is was not possible to make the frame the whole building process fails.
        Otherwise what it is left to initialize on the builder is properly done. */
    builder.file_stream = _OutputStream;
    builder.registries_written = 0;

    /*  (...) */
    if (! fwrite(& builder.header, sizeof(ERBT_Header), 1, _OutputStream)) {
        return false;
    }
    
    // Counters that tracks the reading progress of the input-stream.
    uint32_t regs_read = 0, currentPage = 0;

    // Sinalizes whether there was an error in inserting a registry data into the tree.
    bool insert_failure = false;


    while ((! insert_failure) && 
        ((regs_read = read_regpage(_InputStream, currentPage ++, & page_buffer)) > 0))
    {
        for (uint32_t i = 0; i < regs_read; i ++) {
            reg_ptr.key = page_buffer.reg[i].key;

#if IMPL_LOGGING
            DebugPrintf("Inserting #%u: %d\n",
                (unsigned int) reg_ptr.original_pos,
                reg_ptr.key);
#endif

            if (! ERBT_insert(& builder, & reg_ptr)) {
                insert_failure = false;
                break;
            }

#if IMPL_LOGGING
            /*
            fprintf(debug_stream, "\n");
            DebugPrintf("After inserting:\n", NULL);
            printRedBlackTree(builder.file_stream);
            fprintf(debug_stream, "\n\n");
            */
#endif          

            ERBT_Balance(& builder, builder.registries_written - 1);

#if IMPL_LOGGING
            fprintf(debug_stream, "\n");
            DebugPrintf("After balacing:\n", NULL);
            printRedBlackTree(builder.file_stream);
            fprintf(debug_stream, "\n\n");
#endif

            reg_ptr.original_pos ++;
        }

        if (currentPage > 7) {
#if IMPL_LOGGING
            DebugPrint("currentPage > 7: break\n");
#endif
            break;
        }


    }
    
    freeFrame(& builder.frame);

#if IMPL_LOGGING
    fallDebug();

#if RECORD_ERBT
    __RBT_WRITE(builder.file_stream, builder.registries_written);
    fclose(RBT_PRINTING_STREAM);
#endif // RECORD_ERBT
#endif // IMPL_LOGGING

    return ! insert_failure;
}


/*  EBST by MRT
    ===========

    TODO: Split. as ebst and erbt gained different meanings, 
    it can be fair saying that this source file become
    too convoluted, considering them individually.
    
    Also, the name "ebst" became misleading, once ERBT is a EBST by def.
    One should think about these nomenclatures eventually... */


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
        
        frame_retrieve_index(_InputStream, _Frame, middle / ITENS_PER_PAGE, & page_pointer);

        buffer_node.reg_ptr.original_pos = middle;
        buffer_node.reg_ptr.key = ((regpage_t *) _Frame -> pages)[page_pointer].reg[middle % ITENS_PER_PAGE].key;

        // DebugPrintR("buffer-node: (%d, %d) <pos:%d, key:%d>\n", buffer_node.left, buffer_node.right, buffer_node.reg_ptr.original_pos, buffer_node.reg_ptr.key);

        write_ebstnode(_OutputStream, iterator, & buffer_node);

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
