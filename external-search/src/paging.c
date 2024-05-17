// <paging.c>

#include "paging.h"



// Pages
// -----

/*  From a data file-stream, reads a single registry page, specified by
    its index. Returns the number of registries read that were read onto the page. */
inline size_t read_regpage(FILE * _Stream, uint32_t _Index, regpage_t * _ReturnPage)
{
    // Setting the file pointer to the beggining of the indexed page on the file.
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);
    return fread(_ReturnPage -> reg, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries on the page written. */
inline size_t write_regpage(FILE * _Stream, uint32_t _Index, const regpage_t * _Page)
{
    //
    fseek(_Stream, regpage_pos(_Index), SEEK_SET);
    return fwrite(_Page -> reg, sizeof(registry_t), ITENS_PER_PAGE, _Stream);
}


/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool bnode_write(const b_node * _Node, size_t _Index, FILE * _BTreeStream) {
    // DebugPrint("[%s] %u\n", _Index);
    fseek(_BTreeStream, bnode_pos(_Index), SEEK_SET);
    return fwrite(_Node, sizeof(b_node), 1, _BTreeStream) > 0;
}

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool bnode_read(b_node * _ReturnNode, size_t _Index, FILE * _BTreeStream) {
    // DebugPrint("index: %u\n", _Index);
    fseek(_BTreeStream, bnode_pos(_Index), SEEK_SET);
    return fread(_ReturnNode, sizeof(b_node), 1, _BTreeStream) > 0;
}


/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
inline bool bstar_write(const bstar_node * _Node, size_t _Index, FILE * _BTreeStream) {
    fseek(_BTreeStream, bstarnode_pos(_Index), SEEK_SET);
    return fwrite(_Node, sizeof(bstar_node), 1, _BTreeStream);
}

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
inline bool bstar_read(bstar_node * _ReturnNode, size_t _Index, FILE * _BTreeStream) {
    fseek(_BTreeStream, bstarnode_pos(_Index), SEEK_SET);
    return fread(_ReturnNode, sizeof(bstar_node), 1, _BTreeStream);
}




/* Temporary */

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

/*  */



// Inicializa os valores iniciais do frame.
inline bool
makeFrame(frame_t * _Frame, const size_t _PageSize) { 
    frame_t frame = {
        .first = NULL_INDEX,
        .last = NULL_INDEX,
        .sized = 0,
        .pages = 0
    };
    * _Frame = frame;
    _Frame -> pages = calloc(PAGES_PER_FRAME, _PageSize);

    return _Frame -> pages != NULL;
}

 // Remove uma paǵina usando o sistema de Fila Circular (nesse caso, é uma adaptação da função "desenfileirar")
bool removePage(frame_t * _Frame) {
    if (isFrameEmpty(_Frame))
        return false;

    //
    if (_Frame -> last == _Frame -> first){
        _Frame -> last = NULL_INDEX;
        _Frame -> first = NULL_INDEX;
    }
    //
    else {
        _Frame -> first = (_Frame -> first + 1) % PAGES_PER_FRAME;
    }
    _Frame -> sized --;

    return true;
}

bool addPage_regpage_t(uint32_t num_page, frame_t * _Frame, FILE * _Stream) { // Adiciona uma página nova para o _Frame. Recebe o número da página que será colocada
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! removePage(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }

    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = 0;
        _Frame -> last = 0;
    }

    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);

    // Copying the page into the frame, as well as its index.
    read_regpage(_Stream, num_page, & ((regpage_t *) _Frame -> pages)[_Frame -> last]);
    _Frame -> indexes[_Frame -> last] = num_page;
    
    _Frame -> sized ++;
    return true;
}


bool addPage_b_node(uint32_t _Index, frame_t * _Frame, FILE *_Stream) { // Adiciona uma página nova para o _Frame. Recebe o número da página que será colocada
    // In case the frame is full, the last page in it is removed.
    if (isFrameFull(_Frame)) {
        if (! removePage(_Frame))
            // Fail in removing means an overall fail on the addition process.
            return false;
    }
    
    // If the frame at this point is empty, then
    // the circular-queue indexes are reset.
    if (isFrameEmpty(_Frame)) {
        _Frame -> first = 0;
        _Frame -> last = 0;
    }

    // Otherwise we increase the last pointer circularly.
    // The fall into the first "isFrameFull" verification will also
    // lead to this one.
    else
        _Frame -> last = incr_frame(_Frame -> last);
    
    // Copying the page into the frame, as well as its index.
    bnode_read(& ((b_node *) _Frame -> pages)[_Frame -> last], _Index, _Stream);
    _Frame -> indexes[_Frame -> last] = _Index; 
    
    _Frame -> sized ++;

    // show_bnode_frame(_Frame);
    return true;
}


void show_regpage_frame(const frame_t *_Frame) { // Mostra as páginas do _Frame, com apenas o seus registros e o número da respectiva página
    if(isFrameEmpty(_Frame)){
        return;
    }
    for (uint32_t i = 0; i < PAGES_PER_FRAME; i++) {
        printf("Page %d | (%d)\t", i + 1, _Frame ->indexes[i]);
        if (i == _Frame -> first) {
            printf("<- first");
        } else if (_Frame -> last == i) {
            printf("<- last");
        }

        putchar('\n');

        for(int j=0; j<ITENS_PER_PAGE; j++){
            printf("Reg %d\n", ((regpage_t *)_Frame -> pages)[i].reg[j].key);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

/*  */
void show_bnode_frame(const frame_t *_Frame) {
    if (isFrameEmpty(_Frame))
        return;
    
    for (uint32_t i = 0; i < PAGES_PER_FRAME; i ++) {
        printf("Page %d", i);
        if (i == _Frame -> first) {
            printf("\t<- first");
        } else if (_Frame -> last == i) {
            printf("\t<- last");
        }
        printf("\n%u", _Frame -> indexes[i]);
        PrintBNode(& ((b_node * ) _Frame -> pages)[i]);
    }
    printf("\n\n");
}

/*  */
inline bool 
searchIndexPageInFrame(const frame_t * _Frame, const uint32_t _Index, uint32_t * _ReturnFrameIndex)
{
    if (isFrameEmpty(_Frame))
        return false;

    for (uint32_t i = _Frame -> first;; i = (i + 1) % PAGES_PER_FRAME) {
        if (_Index == _Frame -> indexes[i]) { 
            * _ReturnFrameIndex = i;
            return true;
        }
        if (i == _Frame -> last)
            break;
    }
    return false;
}






/*  */
inline bool bnode_retrieve(b_node * _ReturnNode, frame_t * _Frame, size_t _Index, FILE * _BTreeStream)
{
    DebugPrintR("index: %u\n", (unsigned int) _Index);

    uint32_t frame_index = 0;
    if (searchIndexPageInFrame(_Frame, _Index, & frame_index))
    {
        printf("\tRefresh:\nframe:%u\t%u", (unsigned int) frame_index, (unsigned int) _Frame -> indexes[frame_index]);
        PrintBNode(& ((b_node *) _Frame -> pages)[frame_index]);
        
        /*

            [4, 1, 2, 3, 7]
                   ^
                   |
         frame_index
        */
        /* TODO: consertar dps zé nó
        
        for (uint32_t i = frame_index, j; i < _Frame -> last; i = j) {
            j = incr_frame(i);

            ((b_node *) _Frame -> pages)[i] = ((b_node *) _Frame -> pages)[j];
                _Frame -> indexes[i] = _Frame -> indexes[j];
        }
        
        ((b_node *) _Frame -> pages)[_Frame -> last] = node_buffer;
        _Frame -> indexes[_Frame -> last] = index_buffer;
        
        * _ReturnNode = ((b_node *) _Frame -> pages)[_Frame -> last];
        */
        * _ReturnNode = ((b_node *) _Frame -> pages)[frame_index]; 

        printf("\nReturn node: ");
        PrintBNode(_ReturnNode);
        return true;
    }
    if (! addPage_b_node(_Index, _Frame, _BTreeStream)) {
        printf("\t> addpage\n");
        return false;
    }
    
    printf("\t> pega o último direto msm. last: %u\n", _Frame -> last);
    * _ReturnNode = ((b_node *) _Frame -> pages)[_Frame -> last];
    return true;
}

/*  */
bool bnode_update(const b_node * _WriteNode, frame_t * _Frame, size_t _NodeIndex, FILE * _BTreeStream)
{
    DebugPrintR("index: %u\n", (unsigned int) _NodeIndex);
    
    uint32_t frame_index = 0;

    //If _WriteNode is in the frame, updates the frame also.
    if (searchIndexPageInFrame(_Frame, _NodeIndex, & frame_index)) {

        printf("frame_index: <%u>\n", frame_index);
        ((b_node *) _Frame -> pages)[frame_index] = * _WriteNode;
    }

    //Updating the b_node in file.
    return bnode_write(_WriteNode, _NodeIndex, _BTreeStream);
}


// B-star
