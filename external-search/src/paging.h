// <paging.h>


#ifndef _PAGING_HEADER_
#define _PAGING_HEADER_


#include "commons.h"


/*  How many itens, at its maximum, holds each page. 
    Observe that it is the same for the different types of pages. */
#define ITENS_PER_PAGE		5					



// Pages
// -----

/*  Registries-page. A page in the registries stream. 
    Structurally: an array of registries. */
typedef struct {
    // The registries themselves.
    registry_t reg[ITENS_PER_PAGE];
} regpage_t;

/*  Returns the position at which the registries-page is located on the registries-stream. */
#define regpage_pos(_Index)    (_Index * sizeof(regpage_t))

/*  From a data file-stream, reads a single registry-page, specified by
    its index. Returns the number of registries read that were read onto the page. */
size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage);

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries that had been written. */
size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _WritePage);


/*  The minimum degree is here defined as:
        2t = MAX_ITENS_PER_PAGE + 1
        t = (MAX_ITENS_PER_PAGE + 1) / 2 */
#define BTREE_MINIMUM_DEGREE    ((ITENS_PER_PAGE + 1) >> 1)

/*  */
#define BTREE_MINIMUM_DEGREE_m1 (BTREE_MINIMUM_DEGREE - 1)

/*  A B-Tree node / page. (...)  */
typedef struct {
    /* 1 [bytes], aligning by 1. */
    struct {
        // How many registry-pointers does it have.
        uint8_t item_count  : 7;

        // Indicates whether the node is a leaf.
        bool is_leaf        : 1;
    };

    registry_pointer reg_ptr[ITENS_PER_PAGE];   // Registry-pointers array.
    uint32_t children_ptr[ITENS_PER_PAGE + 1];  // Children pointer array.
} b_node;

/*  Returns the position at which the node given by the passed index is at the
    B-Tree file stream. */
#define bnode_pos(_Index)     (sizeof(b_node) * _Index)

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
bool read_bnode(B_STREAM * _Stream, size_t _NodeIndex, b_node * _ReturnNode);

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
bool write_bnode(B_STREAM * _Stream, size_t _NodeIndex, const b_node * _WriteNode);


/*  A B-star node / page. (...) */
typedef struct {
    // 1 [byte], 1 [byte] alignment.
    struct {
        uint8_t item_count : 7;
        bool    is_leaf : 1;
    };

    /*  (...) */
    union {
        // 8 * ITENS_PER_PAGE + 4 [bytes], 4 [bytes] alignment.
        struct {
            key_t keys[ITENS_PER_PAGE];
            uint32_t children_ptr[ITENS_PER_PAGE + 1];
        } inner;

        // 8 * ITENS_PER_PAGE [bytes], 4 [bytes] alignment.
        struct {
            registry_pointer reg_ptr[ITENS_PER_PAGE];
        } leaf;
    };

    // 8 * ITENS_PER_PAGE + 4 + 1 -> 8 * (ITENS_PER_PAGE + 1) [bytes], 4 [bytes] alignment.
    // 48 [bytes].
} bstar_node;

/*  */
#define bstarnode_pos(_Index)       (_Index * sizeof(bstar_node))


/*  */
bool read_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, bstar_node * _ReturnNode);

/*  */
bool write_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, const bstar_node * _WriteNode);


// The frame
// ---------

// How many page-frames are there to be stored in the pagination-system.
#define PAGES_PER_FRAME		5					

/*  The frame for the pagination-system, polymorphic in respect to the page type.
    Structurally, implements a circular queue into which the pages are added and removed. */
typedef struct {
    /*  For the sake of the page polymorphism of the frame, the pages array is dynamic.
        But invariably, it will always allocate <PAGES_PER_FRAME> elements (pages) in it. */

    // A dynamic array of pages of size <PAGES_PER_FRAME>.
    void * pages;

    // The corresponding indexes for the pages. Array of size <PAGES_PER_FRAME>.
    uint32_t indexes[PAGES_PER_FRAME];

    // Pointer to the initial position of the circular-queue. Inclusive.
    uint32_t first;
    
    // Pointer to the last position of the circular-queue. Inclusive.
    uint32_t last;

    // The size of the frame in pages; it is: "how many pages are load into the frame"?
    uint32_t sized;
} frame_t;  
// TODO: (Refactor) -> Frame? For which it can be considered as an ds object.


#define NULL_INDEX      0

//
#define isFrameFull(_Frame)  (_Frame -> sized == PAGES_PER_FRAME)

//
#define isFrameEmpty(_Frame) (_Frame -> sized == 0)

bool makeFrame(frame_t * _Frame, const size_t _PageSize);

#define freeFrame(_Frame)   free(_Frame.pages);

bool removePage(frame_t *_Frame);

bool addPage_regpage(uint32_t num_page, frame_t *_Frame, REG_STREAM *_Stream);
bool addPage_b_node(uint32_t num_page, frame_t *_Frame, B_STREAM *_Stream);
bool addPage_bstar(uint32_t _Index, frame_t * _Frame, BSTAR_STREAM *_Stream);

void show_regpage_frame(const frame_t* _Frame);

void show_bnode_frame(const frame_t *_Frame);



// builders

/*  (...) */
typedef struct {
    B_STREAM * file_stream;
    uint32_t nodes_qtt;
    frame_t frame;
    b_node root;
} B_Builder;


/*  (...) */
typedef struct {
    BSTAR_STREAM * file_stream;
    uint32_t nodes_qtt;
    frame_t frame;
    bstar_node root;
} BStar_Builder;


/*  */
bool retrieve_regpage(REG_STREAM * _Stream, frame_t * _Frame, size_t _Index, uint32_t * _ReturIndex);



/*  */
bool retrieve_bnode(B_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, b_node * _ReturnNode);

/*  */
bool update_bnode(B_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, const b_node * _WriteNode);



/*  */
bool retrieve_bstar(BSTAR_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, bstar_node * _ReturnNode);

/*  */
bool update_bstar(BSTAR_STREAM * _Stream, frame_t * _Frame, size_t _NodeIndex, const bstar_node * _WriteNode);



/*  */
bool searchIndexPageInFrame(const frame_t * _Frame, const uint32_t _Index, uint32_t * _ReturnFrameIndex);













// loc


/*  */
#define mod_incr(x, m)				((x + 1) % m)

/*  */
#define incr_frame(x)               mod_incr(x, PAGES_PER_FRAME)


// The key of the regpage is simply the key of first one by its itens.
#define regpage_key(p)			(p.reg[0].key)	


#endif  //_PAGING_HEADER_