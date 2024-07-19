/*  <src/include/bstar.h>

    (...) */


#ifndef _ES_BSTAR_COMMON_HEADER_
#define _ES_BSTAR_COMMON_HEADER_


#include "searching.h"


/*  (...) */
typedef struct {
    BSTAR_STREAM * file_stream;
    uint32_t nodes_qtt;
    Frame frame;
    bstar_node root;
} BStar_Builder;

/*	*/
bool BSTree_Build(REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream);

/*	*/
bool BSTree_Search(key_t key, REG_STREAM * _InputStream, BSTAR_STREAM * _OutputStream, Frame * _Frame, registry_t * target);

/*  */
bool BSTree_insert(const registry_pointer * _Reg, BStar_Builder * _bs_builder);



#endif // _ES_BSTAR_COMMON_HEADER_
