/*  <src/paging.h>
    
    Where the all "pages" and their fundamental IO operations are defined. */


#ifndef _ES_PAGING_HEADER_
#define _ES_PAGING_HEADER_


#include "registry.h"
#include "log.h"


/*  TODO: (Implement)
    Defines the size of a buffer page in the application, in bytes. */
#define PAGE_BUFFER_SIZE

/*  How many itens, at its maximum, holds each page. 
    Observe that it is the same for the different types of pages. */
#define ITENS_PER_PAGE		5					



// Files and streams
// -----------------

/*  The type of each type of stream dealt is annotated for hinting arguments. */

typedef FILE	REG_STREAM;     // A stream representing the registries file.
typedef FILE	B_STREAM;       // A stream representing the B tree data-structure.
typedef FILE	BSTAR_STREAM;   // A stream representing the B* tree data-structure.
typedef FILE	EBST_STREAM;    // A stream representing the (pure) ebst data-structure.
typedef FILE	ERBT_STREAM;    // A stream representing the erbt data-structure.


// The default registries filename.
// #define INPUT_DATAFILENAME			"temp/u-100.bin"
#define INPUT_DATAFILENAME			"temp/input-data.bin"

// The default b-tree data-structure filename.
#define OUTPUT_BTREE_FILENAME	    "temp/data.btree"

// The default b*-tree data-structure filename.
#define OUTPUT_BSTAR_FILENAME       "temp/data.bstar"

// The default binary tree data-structure filename.
#define OUTPUT_EBST_FILENAME        "temp/data.ebst"
#define OUTPUT_ERBT_FILENAME		"temp/data.erbt"

// TODO: (Ponder)
#define OUTPUT_ISS_FILENAME         "temp/data.iss"



// Registries page
// ---------------

/*  Registries-page. A page in the registries stream. 
    Structurally: an array of registries. */
typedef struct {
    // The registries themselves.
    registry_t reg[ITENS_PER_PAGE];
} regpage_t;

/*  Returns the position at which the registries-page is located on the registries-stream. */
#define regpage_pos(_Index)    (_Index * sizeof(regpage_t))

/*  The key of the regpage is simply the key of first one by its itens. */
#define regpage_key(p)			(p.reg[0].key)	

/*  From a data file-stream, reads a single registry-page, specified by
    its index. Returns the number of registries read that were read onto the page. */
size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage);

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries that had been written. */
size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _WritePage);

/*  From a data file-stream and a registry reference pointer, attempts retrieving the entire
    registry data from it. Returns success. */
bool search_registry(REG_STREAM * _Stream, const registry_pointer * _Reference, registry_t * _ReturnRegistry);



// B-Tree node
// -----------

/*  The minimum degree is here defined as follows:
    
    As each page can hold, on its maximum, (2t - 1) itens,
    the maximum value of t that doesn't exceed that limit is then
        
        2t - 1 = ITENS_PER_PAGE =>
        2t = ITENS_PER_PAGE + 1 =>
        t = floor{(ITENS_PER_PAGE + 1) / 2}. */
#define BTREE_MINIMUM_DEGREE        ((ITENS_PER_PAGE + 1) >> 1)

/*  Minimum degree minus 1. (t - 1). */
#define BTREE_MINIMUM_DEGREE_m1     (BTREE_MINIMUM_DEGREE - 1)


/*  A B-Tree node/page. 
    Structurally, it contains a counter of how much items does it contains, 
    an indicator of whether it is a leaf, and arrays for the registry pointers 
    and the children pointers. */
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



// B*-Tree node
// ------------

/*  A B* node/page. 
    Structurally, it contains a counter of how much items does it contains,
    an indicator of whether it is a leaf, and arrays for the keys and the 
    children pointers, or an array for the registry pointers, dependending
    upon it being an inner or a leaf node, respectivelly. */
typedef struct {
    // 1 [byte], 1 [byte] alignment.
    struct {
        uint8_t item_count  : 7;
        bool    is_leaf     : 1;
    };

    /*  Recognized differently if it is an inner or a leaf node. */
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
    }; // 8 * ITENS_PER_PAGE + 4 [bytes] total, with a waste of 4 [bytes] if it is a leaf.

    // 8 * ITENS_PER_PAGE + 4 + 1 => 8 * (ITENS_PER_PAGE + 1) [bytes], 4 [bytes] alignment.
    // For ITENS_PER_PAGE = 5, occupies then 48 [bytes].
} bstar_node;

/*  Returns the position at which the node given by the passed index is at the
    B*-Tree file stream. */
#define bstarnode_pos(_Index)       (_Index * sizeof(bstar_node))


/*  Reads a single b* node on the B* tree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
bool read_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, bstar_node * _ReturnNode);

/*  Writes a single b* node on the B* tree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
bool write_bstar(BSTAR_STREAM * _Stream, size_t _NodeIndex, const bstar_node * _WriteNode);



// EBST & ERBT
// -----------

// * Represents a internal pointer in the ebst.
typedef int32_t ebst_ptr;

/*  In the ebst_ptr context, represents a null, invalid index. */
#define EBST_NULL_INDEX		(-1)

/*  In the ebst_ptr context, represents a null, invalid index. 
    (Synonym naming for EBST_NULL_INDEX.) */
#define ERBT_NULL_INDEX     EBST_NULL_INDEX    


/*  A node of a external binary search tree.
    Structurally holds a registry-pointer and two
    directional internal pointers. */
typedef struct {
    // The registry information the node holds.
    registry_pointer reg_ptr;

    // Node's branching, children pointers.
    ebst_ptr left, right;
} ebst_node;


/*  A node of a external red-black tree.
    Structurally a ebst-node, additionally containing
    a pointer to its father and its color. */
typedef struct {
    /*  The ebst-node itself, of whats erbt-node 
        is a abstraction on top.
        
        * Note: declaring it freely like so does the intended
        on windows OS as it seems, but the behavior is that of unrecognition on others.
        Because of that, the proprieties of ebst_node are expanded followingly. */
    // ebst_node;
    registry_pointer reg_ptr;
    ebst_ptr left, right;


    bool color      : 1;    // Determines which color is the node - either red or black.
    ebst_ptr father : 31;   // A pointer to the node's father.
} erbt_node;    // 24 [bytes], 4 of alignment.



typedef struct {
    ebst_ptr root_ptr;
} ERBT_Header;

#define erbt_header_pos()       0
#define erbtnode_pos(_Index)    (sizeof(ERBT_Header) + sizeof(erbt_node) * _Index)

/*  */
bool read_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, ebst_node * _Node);

/*  */
bool write_ebstnode(EBST_STREAM * _Stream, size_t _NodeIndex, const ebst_node * _WriteNode);

/*  */
bool read_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, erbt_node * _ReturnNode);

/*  */
bool write_erbtnode(ERBT_STREAM * _Stream, size_t _NodeIndex, const erbt_node * _WriteNode);



#endif  // _ES_PAGING_HEADER_