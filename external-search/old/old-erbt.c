/*  ERBT balance case 1.1 and 2.1: color correction
      BLACK                                  RED
   <-       ->      == Becomes ==>        <-     ->
RED           RED                   BLACK           BLACK
*/
inline static void
_ERBT_Balance_case_change(struct ERBT_Balancer * balancer) {

    startDebug(); DebugFuncMark();
    // CASE 1.1 & 2.1: Uncle also is RED
    balancer->grandfather_node.color = RED;
    balancer->uncle_node.color = BLACK;
    balancer->father_node.color = BLACK;

    write_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);
    write_erbtnode(balancer->builder->file_stream, balancer->uncle_index, &balancer->uncle_node);
    write_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->father_node);

    // temp
    printRedBlackTree(balancer->builder->file_stream);

    // Grandpa becomes the Main Node
    // balancer -> node.line = balancer -> father_node.father;
    balancer->node_index = balancer->father_node.father;
    read_erbtnode(balancer->builder->file_stream, balancer->node_index, &balancer->node);

    endDebug();
}

/*  ERBT balance case 1.2: Adapts a node-frame for case 1.3.
*/
inline static void
_ERBT_Balance_case1_2(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();
    // CASE 1.2: Main Node is not at the same side as he's father
    rotateLeft(balancer->builder, balancer->node.father);

    // Updates the position, bringing the sequence back to Grandpa -> Son -> Father  
    /*  For passing to case 1.3, the node will now be that that was originally the father...

    */
    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->node);
    read_erbtnode(balancer->builder->file_stream, balancer->node_index, &balancer->father_node);

    balancer->node_index = balancer->node.father; // * not necessary, but this implies in concordance...
    read_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    printf("-------------------------------------\n");
    DebugPrint("After case 1.2:\n", NULL);
    printf("node\t:"); PrintEBSTNode(&balancer->node);
    printf("father:"); PrintEBSTNode(&balancer->father_node);
    printf("grandfather:"); PrintEBSTNode(&balancer->grandfather_node);
    printf("\n\n");
    printRedBlackTree(balancer->builder->file_stream);
    printf("-------------------------------------\n");

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

    DebugPrint("Rotate right for grandfather: %u\n", (unsigned int) balancer->father_node.father);

    // * Note: the positions doesn't change with the rotation, only the pointer in the stream.
    rotateRight(balancer->builder, balancer->father_node.father);

    /*  At this point, the color between the X and Y (on the rotation context)
        have to be switched.

        X corresponds to the grandfather, at the step that Y is the father. */
    DebugPrint("Various transformations...\n", NULL);

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
    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->node);

    printf("-----------------------------------------\n");
    printf("%d, %d, %d\n", balancer->node_index, balancer->node.father, balancer->father_node.father);
    PrintEBSTNode(&balancer->node);
    PrintEBSTNode(&balancer->father_node);
    PrintEBSTNode(&balancer->grandfather_node);
    printRedBlackTree(balancer->builder->file_stream);
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

    rotateRight(balancer->builder, balancer->node.father);

    // Updates the node-information, after the rotation, into the balancer - bringing it to main-memory.
    // read_erbtnode(balancer -> builder -> file_stream, balancer -> node.father, & balancer -> father_node);
    //read_erbtnode(balancer -> builder -> file_stream, balancer -> node_index, & balancer -> node);

    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->node);
    read_erbtnode(balancer->builder->file_stream, balancer->node_index, &balancer->father_node);

    balancer->node_index = balancer->node.father; // * not necessary, but this implies in concordance...
    read_erbtnode(balancer->builder->file_stream, balancer->father_node.father, &balancer->grandfather_node);

    endDebug();
}

inline static void
_ERBT_Balance_case_2_3(struct ERBT_Balancer * balancer) {
    startDebug(); DebugFuncMark();

    DebugPrint("Rotate left for grandfather: %u\n", (unsigned int) balancer->father_node.father);

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
    read_erbtnode(balancer->builder->file_stream, balancer->node.father, &balancer->node);


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

    balancer.node = (erbt_node){ 0 };
    balancer.father_node = (erbt_node){ 0 };
    balancer.uncle_node = (erbt_node){ 0 };
    balancer.grandfather_node = (erbt_node){ 0 };


    //balancer.father_node.line = ERBT_NULL_INDEX;
    balancer.father_node.father = ERBT_NULL_INDEX;

    //balancer.grandfather_node.line = ERBT_NULL_INDEX;
    balancer.grandfather_node.father = ERBT_NULL_INDEX;

    //balancer.uncle_node.line = ERBT_NULL_INDEX;

    // Start Main Node 
    read_erbtnode(_builder->file_stream, balancer.node_index, &balancer.node);

    // Set / update Nodes
    if (balancer.node.father != ERBT_NULL_INDEX) {
        read_erbtnode(_builder->file_stream, balancer.node.father, &balancer.father_node);

        // Start Gradpa if exists
        if (balancer.father_node.father != ERBT_NULL_INDEX) {
            read_erbtnode(_builder->file_stream, balancer.father_node.father, &balancer.grandfather_node);
        }
    }

    DebugPrintY("Before while...\n", NULL);
    printf("node: "); PrintEBSTNode(&balancer.node);
    printf("father: "); PrintEBSTNode(&balancer.father_node);
    printf("grandfather: "); PrintEBSTNode(&balancer.grandfather_node);


    // If: the balancer.node is root or balancer.node is Black or Father is Black, the balancing ends
    /* (...) */
    while ((balancer.node_index != 0) && (balancer.father_node.color == RED) && (balancer.father_node.father != -1))
    {
        DebugPrintY("Inside the while loop...\n\t", NULL);

        // In case the father node is at left,
        if (balancer.node.father == balancer.grandfather_node.left) {
            printf("\t"); DebugPrintY("father is left...\n", NULL);

            // Set main uncle
            if ((balancer.father_node.father != ERBT_NULL_INDEX) && (balancer.grandfather_node.right != ERBT_NULL_INDEX)) {
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
                _ERBT_Balance_case1_3(&balancer);
            }

            // In case the father node is at the right,
        }
        else {
            DebugPrintY("father is right...\n", NULL);

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
                _ERBT_Balance_case_change(&balancer);

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
    }

    // Make sure that the root is BLACK
    erbt_node root;
    read_erbtnode(_builder->file_stream, 0, &root);
    root.color = BLACK;
    write_erbtnode(_builder->file_stream, 0, &root);

    endDebug();
}