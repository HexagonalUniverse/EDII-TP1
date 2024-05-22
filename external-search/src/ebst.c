#include "ebst.h"


// Debug
static void
PrintEBSTNode(const ebst_node * _Node){
    printf("\t| [line:%3.u ", 
        (unsigned int) _Node -> line);
    printf("\t| left:%3.d   key:%3.d   right:%3.d | ",
        (int) _Node->left,
        (int) _Node->reg_ptr.key,
        (int) _Node->right);

    printf("father: %3.d",
        (int) _Node -> father);

    printf("]\n");
}

void printRedBlackTree(EBST_STREAM * _Stream) {
    DebugFuncMark();

    ebst_node tempNode = { 0 };
    uint32_t i = 0;

    while (read_ebstnode(_Stream, i ++, & tempNode)) {    
        if (tempNode.color == RED)
            aec_fg_red();
        else
            aec_fg_white();
        
        PrintEBSTNode(&tempNode);
        aec_reset();
    }
}

static ebst_node EBST_nodeStarter(const registry_pointer * _Entry) {
    ebst_node new_node = {
        .color = RED,
        .reg_ptr = * _Entry,
        .father = -1,
        .left = -1,
        .right = -1,
        .line = -1,
    };

    return new_node;
}

static bool EBST_insertRoot(EBST_Builder * _builder, const registry_pointer * _Entry) {
    DebugFuncMark();

    ebst_node new_node = {
        .color = BLACK,
        .reg_ptr = * _Entry,
        .father = -1,
        .left = -1,
        .right = -1,
        .line = 0,
    };
    _builder -> registries_written ++;
    return write_ebstnode(_builder -> file_stream, 0, & new_node);
}

// Insert the node in the External Memory
static bool EBST_insert(EBST_Builder * _builder, const registry_pointer * _Entry) {
    startDebug();
    DebugFuncMark();

    // First entry on the binary search tree (root node)
    if (_builder -> registries_written == 0)
        return EBST_insertRoot(_builder, _Entry);

    // **
    ebst_node NewNode = EBST_nodeStarter(_Entry);
    ebst_node currentNode = { 0 };
    uint32_t node_index = 0;    // ~ index of currentNode

    bool had_failure = false;
    
    while (read_ebstnode(_builder -> file_stream, node_index, & currentNode)) {

        //  Check left first
        if (NewNode.reg_ptr.key < currentNode.reg_ptr.key) {

            //Check if the current node has a son
            if (currentNode.left == EBST_NULL_INDEX) {
                // Insertion process
                // Update current node, now it points to the end of file
                NewNode.father = currentNode.line;  // == node_index
                currentNode.left = NewNode.line = _builder -> registries_written;

                //  Updating father
                write_ebstnode(_builder -> file_stream, node_index, & currentNode);

                //  Adds new node
                write_ebstnode(_builder -> file_stream, _builder -> registries_written, & NewNode);
                _builder -> registries_written ++;
                break;

            } else
                node_index = currentNode.left;

        //  Check right
        } else if (NewNode.reg_ptr.key > currentNode.reg_ptr.key){

            //Check if the current node has a son
            if (currentNode.right == EBST_NULL_INDEX) {
                // Insertion process
                // Update current node, now it points to the end of file
                NewNode.father = currentNode.line;  // == node_index
                currentNode.right = NewNode.line = _builder -> registries_written;

                //  Updating father
                write_ebstnode(_builder -> file_stream, node_index, & currentNode);

                //  Adds new node
                write_ebstnode(_builder -> file_stream, _builder -> registries_written, & NewNode);
                _builder -> registries_written ++;
                break;

            } else
                node_index = currentNode.right;

        // Nodes are equal, that means a fail
        } else {
            DebugPrintR("Error: nodes are equal...\n", NULL);
            had_failure = true;
            break;  
        }
    }
    endDebug();

    if (had_failure) {
        DebugPrintR("Insertion Error: something went wrong...\n",NULL);
        return false;
    }
    return true;
}


#define BALANCE true

#if BALANCE

/// RedBlackTree exclusive Functions
// Left rotation
static void 
rotateLeft(EBST_Builder * _builder, uint32_t _targetLine) {
    DebugFuncMark();
    
    ebst_node X, Y, tempNode;

    int TemporaryPosition;
    /// Set Nodes
    // Start X  
    read_ebstnode(_builder -> file_stream, _targetLine, & X);
    // Start Y (X's right son)
    read_ebstnode(_builder -> file_stream, X.right, &Y);
    
    /// Operations
    // Y-left sub tree becomes X-right sub tree
    X.right = Y.left;
    // Updates Y's father (same as X)
    Y.father = X.father;
    // Changes X and Y position
    TemporaryPosition = Y.line;
    Y.line = X.line;
    X.line = TemporaryPosition;
    // Puts X above Y
    Y.left = X.line;
    // Fix the relation
    X.father = Y.line;

    // Updates X's right son
    if (X.right != -1) {
        read_ebstnode(_builder -> file_stream, X.right, &tempNode);
        tempNode.father = X.line;
        write_ebstnode(_builder -> file_stream, X.right, &tempNode);
    }

    // Updates X's left son 
    if (X.left != -1) {
        read_ebstnode(_builder -> file_stream, X.left, &tempNode);
        tempNode.father = X.line;
        write_ebstnode(_builder -> file_stream, X.left, &tempNode);
    }

    // Update file
    write_ebstnode(_builder -> file_stream, X.line, & X);
    write_ebstnode(_builder -> file_stream, Y.line, & Y);
}

// Right rotation
static void 
rotateRight(EBST_Builder * _builder, uint32_t _targetLine) {
    DebugFuncMark();

    ebst_node X, Y, tempNode;
    int TemporaryPosition;
    
    /// Set Nodes
    // Start X
    read_ebstnode(_builder -> file_stream, _targetLine, & X);
    // Start Y
    read_ebstnode(_builder -> file_stream, X.left, & Y);
    
    
    // Y-Right sub tree becomes X-Left sub tree
    X.left = Y.right;
    // Updates Y's father (same as X)
    Y.father = X.father; 
    // Changes X and Y position
    TemporaryPosition = Y.line;
    Y.line = X.line;
    X.line = TemporaryPosition;    
    // Puts X above Y
    Y.right = X.line;
    // Fix the relation
    X.father = Y.line;


    if (X.right != -1) {
        read_ebstnode(_builder -> file_stream, X.right, & tempNode);
        tempNode.father = X.line;
        write_ebstnode(_builder -> file_stream, X.right, & tempNode);
    }
    // Updates X's left son 
    if (X.left != -1) {
        read_ebstnode(_builder -> file_stream, X.left, &tempNode);
        tempNode.father = X.line;
        write_ebstnode(_builder -> file_stream, X.left, &tempNode);
    }
    //Update file
    write_ebstnode(_builder->file_stream, X.line, &X);
    write_ebstnode(_builder->file_stream, Y.line, &Y);

}



inline static void 
_EBST_Balance_case_change(EBST_Builder * _builder, ebst_node * _node, ebst_node * _uncle, ebst_node * _father, ebst_node * _grandfather){
    startDebug(); DebugFuncMark();
    // CASE 1.1 & 2.1: Uncle also is RED
    _grandfather -> color   = RED;
    _uncle -> color         = BLACK;
    _father -> color        = BLACK;
    write_ebstnode(_builder -> file_stream, _grandfather -> line, _grandfather);
    write_ebstnode(_builder -> file_stream, _uncle -> line, _uncle);
    write_ebstnode(_builder -> file_stream, _father -> line, _father);

    // temp
    printRedBlackTree(_builder->file_stream);

    // Grandpa becomes the Main Node
    _node -> line = _grandfather -> line;
    read_ebstnode(_builder -> file_stream, _node -> line, _node);

    endDebug();
}

inline static void
_EBST_Balance_case1_2(EBST_Builder * _builder, ebst_node * _node, ebst_node * _father, ebst_node * _grandfather){
    startDebug(); DebugFuncMark();
    // CASE 1.2: Main Node is not at the same side as he's father
    // Saves the position before the rotation, because the sequence: Grampa -> Father -> Son will change to Grandpa -> Son -> Father
    int temp = _father -> line;
    _father -> line = _node -> line;
    _node -> line = temp;
    rotateLeft(_builder, temp);

    // Updates the position, bringing the sequence back to Grandpa -> Son -> Father                    
    read_ebstnode(_builder -> file_stream, _father -> line, _father);
    read_ebstnode(_builder ->file_stream, _node -> line, _node);
    // Fix Relations
    _father -> father = _grandfather -> line;
    // Update file
    write_ebstnode(_builder -> file_stream, _father -> line, _father);

    endDebug();
}

inline static void
_EBST_Balance_case1_3(EBST_Builder * _builder, ebst_node * _node, ebst_node * _father, ebst_node * _grandfather) {
    startDebug(); DebugFuncMark();
    //  CASE 1.3: MainNode is at the correct side
    _node -> father = _grandfather -> line;
    write_ebstnode(_builder -> file_stream, _node -> line, _node);

    rotateRight(_builder, _grandfather -> line);
    read_ebstnode(_builder -> file_stream, _grandfather -> line, _grandfather); // Grandpa Become Brother
    read_ebstnode(_builder -> file_stream, _father -> line, _father);  // Stays father, but in grandpa's positions

    /*
    _grandfather->color = BLACK;
    _father->color = RED;
    */
    bool temp = _grandfather -> color;
    _grandfather -> color = _father -> color;
    _father -> color = temp;
  
    write_ebstnode(_builder -> file_stream, _father -> line, _father);
    write_ebstnode(_builder -> file_stream, _grandfather -> line, _grandfather);

    _node -> line = _grandfather -> line;     // Main node is now grandpa
    read_ebstnode(_builder -> file_stream, _node -> line, _node);

    endDebug();
}

inline static void
_EBST_Balance_case2_2(EBST_Builder * _builder, ebst_node * _node, ebst_node * _father, ebst_node * _grandfather){
    startDebug(); DebugFuncMark();
    // Saves the position before the rotation, because the sequence: Grampa -> Father -> Son will change to Grandpa -> Son -> Father
    int temp = _father -> line;
    _father -> line = _node -> line;
    _node -> line = temp;
    rotateRight(_builder, temp);
    // Updates the position, bringing the sequence back to Grandpa -> Son -> Father                    
    read_ebstnode(_builder -> file_stream, _father -> line, _father);
    read_ebstnode(_builder -> file_stream, _node -> line, _node);
    // Fix Relations
    _father -> father = _grandfather -> line;
    // Update file
    write_ebstnode(_builder -> file_stream, _father -> line, _father);

    endDebug();
}


inline static void
_EBST_Balance_case_2_3(EBST_Builder * _builder, ebst_node * _node, ebst_node * _father, ebst_node * _grandfather) {
    startDebug(); DebugFuncMark();
    _node -> father = _grandfather -> line;
    write_ebstnode(_builder -> file_stream, _node -> line, _node);

    rotateLeft(_builder, _grandfather -> line);

    read_ebstnode(_builder -> file_stream, _grandfather -> line, _grandfather);
    read_ebstnode(_builder -> file_stream, _father -> line, _father);

/*
    _grandfather -> color = BLACK;
    _father -> color = RED;
*/
    bool temp = _grandfather -> color;
    _grandfather -> color = _father -> color;
    _father -> color = temp;
    
    write_ebstnode(_builder -> file_stream, _father -> line, _father);
    write_ebstnode(_builder -> file_stream, _grandfather -> line, _grandfather);

    _node -> line = _grandfather -> line;
    read_ebstnode(_builder -> file_stream, _node -> line, _node);

    endDebug();
}


// Balancing the RedBlack Tree
static void EBST_Balance(EBST_Builder * _builder, uint32_t _Xline) {
    startDebug(); DebugPrint("_XLine: %u\n", (unsigned int) _Xline);

    ebst_node 
        MainNode = { 0 },
        MainFather = { 0 }, 
        MainGrandpa = { 0 }, 
        MainUncle = { 0 };

    MainFather.line = -2;
    MainFather.father = -1;

    MainGrandpa.line = -2;
    MainGrandpa.father = -1;

    // Start Main Node 
    read_ebstnode(_builder -> file_stream, _Xline, & MainNode);

    // Set/update Nodes
    if (MainNode.father != -1) {
        read_ebstnode(_builder -> file_stream, MainNode.father, &MainFather);
        
        // Start Gradpa if exists
        if (MainFather.father != -1) { 
            read_ebstnode(_builder -> file_stream, MainFather.father, &MainGrandpa);
        }
    } 

    DebugPrintY("Before while...\n", NULL);

    // If: the MainNode is root or MainNode is Black or Father is Black, the balancing ends
    while ((MainNode.line != 0) && (MainNode.color == RED) && (MainFather.color == RED))
    {
        DebugPrintY("Inside the while loop...\n\t", NULL);
        PrintEBSTNode(& MainNode);

        // MainFather is at left
        if (MainFather.line == MainGrandpa.left) {
            printf("\t"); DebugPrintY("father is left...\n", NULL);

            //Set main uncle
            if ((MainGrandpa.line != -2) && (MainGrandpa.right != -1))
                read_ebstnode(_builder -> file_stream, MainGrandpa.right, & MainUncle);
            
            else {
                MainUncle = (ebst_node) { 0 };
                MainUncle.line = -2;
                MainUncle.father = -1;
            }
            
            // CASE 1.1 Uncle is RED
            if ((MainUncle.line != -2) && (MainUncle.color == RED))
                _EBST_Balance_case_change(_builder, & MainNode, & MainUncle, & MainFather, & MainGrandpa);
            
            else {
                // CASE 1.2: Main Node is not at the same side as he's father
                if (MainNode.line == MainFather.right) {
                    _EBST_Balance_case1_2(_builder, & MainNode, & MainFather, & MainGrandpa);
                }
                // CASE 1.3: Main Node is at the correct side
                _EBST_Balance_case1_3(_builder, & MainNode, & MainFather, & MainGrandpa);
            }
        
        // MainFather is at right
        } else {
            DebugPrintY("father is right...\n", NULL);

            // Set MainUncle
            if ((MainGrandpa.line != -2) && (MainGrandpa.left != -1))
                read_ebstnode(_builder -> file_stream, MainGrandpa.left, & MainUncle);

            else {
                MainUncle = (ebst_node) { 0 };
                MainUncle.line = -2;
                MainUncle.father = -1;
            }
            // CASE 2.1 Uncle is RED
            if ((MainUncle.line != -2) && (MainUncle.color == RED)) {
                _EBST_Balance_case_change(_builder, & MainNode, & MainUncle, & MainFather, & MainGrandpa);

            } else {
                // CASE 2.2: Main Node is not at the same side as he's father
                if (MainNode.line == MainFather.left)
                    _EBST_Balance_case2_2(_builder, & MainNode, & MainFather, & MainGrandpa);
                
                // CASE 2.3: Main Node is at the correct side
                _EBST_Balance_case_2_3(_builder, & MainNode, & MainFather, & MainGrandpa);

            }
        }

        // Set/update Nodes
        if (MainNode.father != -1) {
            read_ebstnode(_builder -> file_stream, MainNode.father, &MainFather);
            if(MainFather.father != -1){
                read_ebstnode(_builder -> file_stream, MainFather.father, &MainGrandpa);
            } else {
                MainGrandpa = (ebst_node) { 0 };
                MainGrandpa.line = -2;
            }
        } else {
            MainFather = (ebst_node) { 0 };
            MainFather.line = -2;
            MainGrandpa = (ebst_node) { 0 };
            MainGrandpa.line = -2;
        
        }
    }
    
    // Make sure that the root is BLACK
    read_ebstnode(_builder -> file_stream, 0, & MainNode);
    MainNode.color = BLACK;
    write_ebstnode(_builder -> file_stream, MainNode.line, & MainNode);
    
    endDebug();
}
#endif

// Generates the file of the binary search tree + calls the balance function
bool EBST_Build(REG_STREAM * _InputStream, EBST_STREAM * _OutputStream) {

    regpage_t page_buffer   = { 0 };
    EBST_Builder builder    = { 0 };
    registry_pointer reg_ptr = { 0 };

    if (! makeFrame(& builder.frame, sizeof(ebst_node)))
        return false;
    
    builder.file_stream = _OutputStream;
    builder.registries_written = 0;
    uint32_t regs_read = 0, currentPage = 0;
    
    bool insert_failure = false;
    
    /*  */
    while ((! insert_failure) && ((regs_read = read_regpage(_InputStream, currentPage ++, & page_buffer)) > 0)) 
    {
        DebugPrintR("Loop, page: #%u\n", (unsigned int) currentPage);

        for (uint32_t i = 0; i < regs_read; i ++) {
            reg_ptr.key = page_buffer.reg[i].key;

            printf("reg <%u, %d>\n",
                reg_ptr.key, reg_ptr.original_pos);
            fflush(stdout);
            
            if (! EBST_insert(& builder, & reg_ptr)) {
                insert_failure = false;
                break;
            }
            EBST_Balance(& builder, builder.registries_written - 1);

            printRedBlackTree(builder.file_stream);

            reg_ptr.original_pos ++;
        }
    }
    // freeFrame(.frame);
    return ! insert_failure;
}
