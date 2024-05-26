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

inline static bool
_ERBT_insert_root(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    DebugFuncMark();

    erbt_node new_node = {
        .color = BLACK,
        .reg_ptr = *_Entry,
        .father = -1,
        .left = -1,
        .right = -1,
    };
    
    _builder -> registries_written ++;
    return write_erbtnode(_builder -> file_stream, 0, &new_node);
}

static bool ERBT_insert(ERBT_Builder * _builder, const registry_pointer * _Entry) {
    startDebug(); DebugFuncMark();

    // Insert Root
    if(_builder -> registries_written == 0)
        return _ERBT_insert_root(_builder, _Entry);

    // Insert New Node
    erbt_node newNode = ERBT_nodeStarter(_Entry);
    
    erbt_node currentNode = { 0 }; 
    uint32_t node_index = _builder -> header.root;
    bool had_failure = true;

    while (read_erbtnode(_builder -> file_stream, node_index, &currentNode)){
        had_failure = false;

        if(newNode.reg_ptr.key < currentNode.reg_ptr.key){
            if(currentNode.left == EBST_NULL_INDEX){
                
                newNode.father = node_index;
                currentNode.left = _builder -> registries_written;

                if (! write_erbtnode(_builder -> file_stream, node_index, & currentNode))
                    had_failure = true;
                
                if (! write_erbtnode(_builder -> file_stream, _builder -> registries_written, & newNode))
                    had_failure = true;

                _builder -> registries_written ++;
                break;
            }
            node_index = currentNode.left;

        } else if (newNode.reg_ptr.key > currentNode.reg_ptr.key){
            if(currentNode.right == EBST_NULL_INDEX){
                newNode.father = node_index;
                currentNode.right = _builder -> registries_written;

                if (! write_erbtnode(_builder -> file_stream, node_index, & currentNode))
                    had_failure = true;

                if (! write_erbtnode(_builder -> file_stream, _builder -> registries_written, & newNode))
                    had_failure = trua;

                _builder -> registries_written++:
                break;
            }
            node_index = currentNode.right;
        } else {
            DebugPrintR("Error: nodes are equal...\n", NULL);
            had_failure = true;
            break;
        }


    }


    endDebug();
}