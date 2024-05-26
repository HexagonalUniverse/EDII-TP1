/*  <src/frame.h>

    Where the LRU Frame data-structure, as a pagination-system, is declared. 
    
    Note that its defintion is polymorphic, in the sense that it does not depend
    specifically directly upon the page it works with. 
    
    Although "frames" are what are stored in this page manager system,
    the whole data-structure is hereby called "the frame". 
    
    "The frame" is, implementatively, a polymorphic circular queue for storing the pages. */


#ifndef _ES_FRAME_HEADER_
#define _ES_FRAME_HEADER_


#include <stdlib.h>     // For dynamic memory allocation functions.
#include "paging.h"


#ifndef PAGES_PER_FRAME

    // How many page-frames are there to be stored in the pagination-system.
    #define PAGES_PER_FRAME		5					

#endif // PAGES_PER_FRAME


// A type-identifier for the pages in the pagination system.
typedef enum {
    B_PAGE,
    BSTAR_PAGE,
    REG_PAGE,
    EBST_PAGE,
    ERBT_PAGE
} page_type;


/*  The frame for the pagination-system, polymorphic in respect to the page type.
    Structurally, implements a circular queue into which the pages are added and removed. */
typedef struct {
    /*  For the sake of the page polymorphism of the frame, the pages array is dynamic.
        But invariably, it will always allocate <PAGES_PER_FRAME> elements (pages) in it. */

    // A dynamic array of pages of size <PAGES_PER_FRAME>.
    void * pages;

    // * page info vv
        uint32_t page_size;
        page_type type;
    // ^^

    // The corresponding indexes for the pages. Array of size <PAGES_PER_FRAME>.
    uint32_t indexes[PAGES_PER_FRAME];

    // Pointer to the initial position of the circular-queue. Inclusive.
    uint32_t first;

    // Pointer to the last position of the circular-queue. Inclusive.
    uint32_t last;

    // The size of the frame in pages; it is: "how many pages are load into the frame"?
    uint32_t size;
} frame_t;
// TODO: (Refactor) -> Frame? For which it can be considered as an ds object.

/*  Increments a pointer in the frame context. It is (x + 1) % PAGES_PER_FRAME. */
#define incr_frame(x)               mod_incr(x, PAGES_PER_FRAME)

#define FRAME_NULL_INDEX      0

// Tells whether the frame (by ref.) is full.
#define isFrameFull(_FramePtr)  (_FramePtr -> size == PAGES_PER_FRAME)

// Tells whether the frame (by ref.) is empty.
#define isFrameEmpty(_FramePtr) (_FramePtr -> size == 0)


/*  The frame data-structure constructor.
    Allocates the frame by reference. Returns success. */
bool frame_make(frame_t * _Frame, const size_t _PageSize, page_type _Type);

/*  Deconstructs (deallocates) the frame. */
#define freeFrame(_Frame)   free(_Frame.pages);

/*  Attempts finding the frame-index at which a given page index is. 
    Returns whether it could find it. */
bool frame_search_index(const frame_t * _Frame, const uint32_t _Index, uint32_t * _ReturnFrameIndex);

/*  Attempts removing the last page in the queue of the frame.
    Returns whether the remotion was possible - it is not when the frame is empty.

    As the process is logical in terms of its internal pointers, the "removed" element
    will persist into memory as garbage. */
bool frame_remove(frame_t * _Frame);

/*  Attempts adding a page to the frame by reference (of its index). 
    Return whether the addition was possible. 
    
    (Does not verify if it is already on the frame.) */
bool frame_add(uint32_t _PageIndex, frame_t * _Frame, FILE * _Stream);

/*  Attempts retrieving a page, given its index, from the frame. In case it is
    already in it, it is refreshed in the frame. Otherwise it's simply added.
    Both copies into the _ReturnPage buffer the page. 

    In case of fail, _ReturnPage is not altered.
    Attent to what the _ReturnPage have be of the type that is specified on the frame. */
bool frame_retrieve_page(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, void * _ReturnPage);

/*  Attempts retrieving an index inside the frame for a page, given its index. 
    Works similarly to frame_retrieve_page, but instead of copying to a specified buffer, 
    retrieves the index to be used in the frame "pages" buffer. 
    
    In case of fail, _ReturnIndex is not altered. */
bool frame_retrieve_index(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, uint32_t * _ReturnIndex);

/*  Attempts updating a page - it is: writing it both the frame and the stream appropriately, 
    so they match accordingly. In case of not existing in the frame, it is added; otherwise
    it is refreshed. Then it is written into the stream.

    Attent to what the _ReturnPage have be of the type that is specified on the frame. */
bool frame_update_page(FILE * _Stream, frame_t * _Frame, uint32_t _PageIndex, const void * _WritePage);



// below, old stuff

void show_regpage_frame(const frame_t * _Frame);

void show_bnode_frame(const frame_t * _Frame);



#endif // _ES_FRAME_HEADER_
