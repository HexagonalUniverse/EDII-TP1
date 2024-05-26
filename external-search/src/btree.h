/*  <src/btree.h> */

#ifndef _ES_BTREE_HEADER_
#define _ES_BTREE_HEADER_


#include "searching.h"


#define INSERT_NON_FULL_ITERATIVE   true


/*  */
bool BTree_Build(REG_STREAM *, B_STREAM *);

/*  */
bool BTree_Search(key_t key, REG_STREAM * _InputStream, B_STREAM * _OutputStream, frame_t * _Frame, registry_t * target);



/*  (...) */
typedef struct {
    B_STREAM * file_stream;
    uint32_t nodes_qtt;
    frame_t frame;
    b_node root;
} B_Builder;

#endif // _ES_BTREE_HEADER_

