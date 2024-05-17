#ifndef _BSTAR_COMMON_HEADER_
#define _BSTAR_COMMON_HEADER_


#include "paging.h"


typedef struct {
    bstar_node root;
} BSTree;



bool BSTree_SplitChild(bstar_node * x, const size_t _Index, BTreeStream * _BTreeStream);

bstar_node  BSTree_SplitRoot(BSTree * _Tree, BTreeStream * _BTreeStream);

bool BSTree_insertNonFull(bstar_node * x, const size_t _XIndex, const registry_pointer * _Reg, BTreeStream * _BTreeStream);

bool BSTree_insert(const registry_pointer * _Reg, BSTree * _Tree, BTreeStream * _BTreeStream);

bool BSTree_Build(FILE * _InputStream, FILE * _OutputStream, frame_t * _Frame);

bool BSTree_Search(key_t key, BTreeStream * _BTreeStream, FILE * _InputStream, registry_t * target);

#endif // _BSTAR_COMMON_HEADER_
