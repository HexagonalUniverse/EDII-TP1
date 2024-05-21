#ifndef _BSTAR_COMMON_HEADER_
#define _BSTAR_COMMON_HEADER_


#include "paging.h"


/*
bool BSTree_SplitChild(bstar_node * x, const size_t _Index, BStar_Builder * _bs_builder);

bstar_node  BSTree_SplitRoot(BStar_Builder * _bs_builder);

bool BSTree_insertNonFull(bstar_node * x, const size_t _XIndex, const registry_pointer * _Reg, BStar_Builder * _bs_builder);

bool BSTree_insert(const registry_pointer * _Reg, BStar_Builder * _bs_builder);
*/

/*	*/
bool BSTree_Build(REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream);

/*	*/
bool BSTree_Search(key_t key, REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream, frame_t * _Frame, registry_t * target);

/*  (...) */
typedef struct {
    BSTAR_STREAM * file_stream;
    uint32_t nodes_qtt;
    frame_t frame;
    bstar_node root;
} BStar_Builder;


#endif // _BSTAR_COMMON_HEADER_
