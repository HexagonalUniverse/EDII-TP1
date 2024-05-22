

#ifndef _EBST_HEADER_
#define _EBST_HEADER_

#include "common.h"


// EBST stands for "External Binary Search Tree".


// Represents a node in the ebst ds file.
typedef struct {
	registry_t root_item;
	
	// Specifies an inner reference on the 
	// ebst ds file for the root directinal child.
	int left, right;
} erbt_node;


#define midpoint(l, r)			(l + ((r - l) >> 1))

#define rightSubfileSize(l, r)	((r - l) - ((r - l) >> 1))


bool assembleEBST_ordered(filepath orderedInputFilename, filepath outputFilename);

#endif // _EBST_HEADER_

