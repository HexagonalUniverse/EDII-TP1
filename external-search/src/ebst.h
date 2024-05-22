#ifndef EBST_H
#define EBST_H


#include "paging.h"

#define RED 0
#define BLACK 1



typedef struct {
	EBST_STREAM * file_stream;
	
	//
	uint32_t registries_written;

	frame_t frame;

} ERBT_Builder;


#define EBST_NULL_INDEX		(-1)


// Generates the file of the binary search tree + calls the balance function
bool EBST_Build(REG_STREAM *, EBST_STREAM *);
void printRedBlackTree(EBST_STREAM * _Stream);

#endif //EBST_H