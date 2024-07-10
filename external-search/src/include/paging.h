/*  <src/paging.h>
    
    Where the all "pages" and their fundamental IO operations are defined. */


/*  TODO: Way too convoluted header file... 
    To move stuff out. */


#ifndef _ES_PAGING_HEADER_
#define _ES_PAGING_HEADER_


#include "registry.h"   // includes "commons.h"...
#include "log.h"


// Global Buffering
// ================

#if ! defined(PAGE_BUFFER_SIZE)
    /*  PAGE_BUFFER_SIZE has to, on the minimum, to be able to store a single item
        in it, of any page. So it has inherently to be bigger or equal than the maximum
        of them all. */
    
    /*  Defines the size in bytes of a page buffer in the application.
        In other words, the overall size of a page is concretly near this value.
        Default as 32 [kB]. */
    #define PAGE_BUFFER_SIZE        32768u
#endif

/*  How many itens, at its maximum, holds each page. 
    Observe that it is the same for the different types of pages. */
#define ITENS_PER_PAGE              5u				


/*  As both B and B* trees actuate by reference, currently,
    it turns out it is convenient considering their buffer size apart.
    If not so, the number of itens in each node would be so tremendously
    large that it won't make sense as a tree. */
#if ! defined(BNODE_BUFFER_SIZE)

    /*  Define the size in bytes of the B tree's node.
        Default as 512 [B]. */
    #define BNODE_BUFFER_SIZE       512u
#endif

#if ! defined(BSNODE_BUFFER_SIZE)
    
    /*  Defin the size in bytes of the B * tree's node.
        Default as 344 [B]. */
    #define BSNODE_BUFFER_SIZE      344u
#endif


// Files and Streams
// =================

/*  The type of each type of stream dealt is annotated for hinting arguments. */

typedef FILE REG_STREAM;    // A stream representing the registries file.
typedef FILE B_STREAM;      // A stream representing the B tree data-structure.
typedef FILE BSTAR_STREAM;  // A stream representing the B* tree data-structure.
typedef FILE EBST_STREAM;   // A stream representing the (pure) ebst data-structure.
typedef FILE ERBT_STREAM;   // A stream representing the erbt data-structure.


// The default registries data file filename. 
// Entry data for the <pesquisa.c>.
#define INPUT_DATAFILENAME			"temp/input-data.bin"   

#define OUTPUT_BTREE_FILENAME	    "temp/data.btree"       // The cache default B tree data-structure filename.
#define OUTPUT_BSTAR_FILENAME       "temp/data.bstar"       // The cache default B* tree data-structure filename.
#define OUTPUT_EBST_FILENAME        "temp/data.ebst"        // The cache default External Binary Search Tree data-structure filename.
#define OUTPUT_ERBT_FILENAME		"temp/data.erbt"        // The cache default External Red-black Tree data-structure filename.
#define OUTPUT_ISS_FILENAME         "temp/data.iss"         // The cache default Index Table data-structure filename.


// Page Types
// ==========

// Registries
// ----------

/*  How many registries, at its maximum, holds each registry-page.
    Default as 5 [u].*/
#define REGPAGE_ITENS           (PAGE_BUFFER_SIZE / sizeof(registry_t))

/*  Registries page: a page in the registries stream.
    Structurally: an array of registries. */
typedef struct { registry_t reg[REGPAGE_ITENS]; } regpage_t;


/*  Returns the position at which the registries-page is located on the registries-stream. */
#define regpage_pos(_Index)     (_Index * sizeof(regpage_t))

/*  The key of the regpage is simply the key of first one by its itens. */
#define regpage_key(p)	        (p.reg[0].key)	

/*  From a data file-stream, reads a single registry-page, specified by
    its index. Returns the number of registries read that were read onto the page. */
size_t read_regpage(REG_STREAM * _Stream, uint32_t _Index, regpage_t * _ReturnPage);

/*  From a data file-stream, writes a single registry-page, specified by
    its index. Returns the number of registries that had been written. */
size_t write_regpage(REG_STREAM * _Stream, uint32_t _Index, const regpage_t * _WritePage);

/*  From a data file-stream and a registry reference pointer, attempts retrieving the entire
    registry data from it. Returns success. */
bool search_registry(REG_STREAM * _Stream, const registry_pointer * _Reference, registry_t * _ReturnRegistry);


// B tree node
// -----------

// Represents a pointer in B and B* Tree data-structures streams.
typedef uint32_t b_ptr;

// OBS: Note that BTREE_ITENS is calculated already infering its size;
// so one should be careful in changing the structure.

/*  (Analysis)

    |registry-pointer| * X + |b_ptr| * (X + 1) + 4 <= PAGE_BUFFER_SIZE
    => 8X + 4X + 4 + 4 <= PAGE_BUFFER_SIZE
    => 12X <= PAGE_BUFFER_SIZE - 8
    therefore X' = max X = floor{(PAGE_BUFFER_SIZE - 4 - sizeof(b_ptr)) / (|registry_pointer| + |b_ptr|)}.

    If we instead hold the registries themselves, instead of pointers to them:
        |reg|X + 4X + 8 <= BS   =>  X <= (BS - 8) // 6020 = 5.   { BS = 32 [kB] } */

/*  How many registry-pointers, at its maximum, holds each b node.
    * Attent to what, currently, it is calculated based on BNODE_BUFFER_SIZE and not PAGE_BUFFER_SIZE.
    
    Default as 42 [u]. */
#define BTREE_ITENS                 ((BNODE_BUFFER_SIZE - 4u - sizeof(b_ptr)) / (sizeof(registry_pointer) + sizeof(b_ptr)))

/*  The minimum degree is here defined as follows:
    
    As each page can hold, on its maximum, (2t - 1) itens,
    the maximum value of t that doesn't exceed that limit is then
        
        2t - 1 = ITENS_PER_PAGE =>
        2t = ITENS_PER_PAGE + 1 =>
        t = floor{(ITENS_PER_PAGE + 1) / 2}. */
#define BTREE_MINIMUM_DEGREE        ((BTREE_ITENS + 1u) >> 1)

/*  Minimum degree minus 1. (t - 1). */
#define BTREE_MINIMUM_DEGREE_m1     (BTREE_MINIMUM_DEGREE - 1)


/*  A B Tree node/page. 
    Structurally, it contains a counter of how much items does it contains, 
    an indicator of whether it is a leaf, and arrays for the registry pointers 
    and the children pointers. */
typedef struct {
    // 4 [B], aligning by 4 [B].
    struct {
        // <is_leaf> field is of uint32_t and bool for compacting reasons.

        uint32_t is_leaf    : 1;    // Indicates whether the node is a leaf.
        uint32_t item_count : 31;   // How many registry-pointers does it have.
    };

    registry_pointer reg_ptr[BTREE_ITENS];   // Registry-pointers array.
    b_ptr children_ptr[BTREE_ITENS + 1];     // B children nodes pointers array.
} b_node;

/*  Returns the position at which the node given by the passed index is at the
    B Tree file stream. */
#define bnode_pos(_Index)     (sizeof(b_node) * _Index)

/*  Reads a single b-node on the BTree data stream, given its index. Returns whether the reading
    was successful - so the node was read on its entirety. */
bool read_bnode(B_STREAM * _Stream, size_t _NodeIndex, b_node * _ReturnNode);

/*  Writes a single b-node on the BTree data stream, given its index. Returns whether the writing
    was successful - so the node was written on its entirety. */
bool write_bnode(B_STREAM * _Stream, size_t _NodeIndex, const b_node * _WriteNode);



// B* Tree node
// ------------

/*  (Analysis)
    
    |b*node|    = 4 + max{|key_t|X + |b_ptr|(X + 1), |registry_pointer|X}
                = 4 + max{(|key_t| + |b_ptr|)X + |b_ptr|, |registry_pointer|X}
                = (|key_t| + |b_ptr|)X + 4 + |b_ptr|

    therefore X = floor{(|b*node| - 4 - |b_ptr|) / (|key_t| + |b_ptr|)}.
*/


/*  Default as 42 [u]. */
#define BSTREE_ITENS                ((BSNODE_BUFFER_SIZE - 4u - sizeof(b_ptr)) / (sizeof(key_t) + sizeof(b_ptr)))
#define BSTREE_MINIMUM_DEGREE       ((BSTREE_ITENS + 1u) >> 1)
#define BSTREE_MINIMUM_DEGREE_m1    (BSTREE_MINIMUM_DEGREE - 1)

/*  A B* node/page. 
    Structurally, it contains a counter of how much items does it contains,
    an indicator of whether it is a leaf, and arrays for the keys and the 
    children pointers, or an array for the registry pointers, dependending
    upon it being an inner or a leaf node, respectivelly. */
typedef struct {
    // 4 [B], 4 [B] alignment.
    struct {
        // <is_leaf> field is of uint32_t and bool for compacting reasons.

        uint32_t is_leaf    : 1;
        uint32_t item_count : 7;
    };

    /*  Recognized differently if it is an inner or a leaf node. */
    union {
        struct {
            key_t keys[BSTREE_ITENS];
            b_ptr children_ptr[BSTREE_ITENS + 1];
        } inner;

        struct {
            registry_pointer reg_ptr[BSTREE_ITENS];
        } leaf;
    };

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
} erbt_node;    // 24 [B], 4 of alignment.



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