#ifndef _ES_EBST_HEADER_
#define _ES_EBST_HEADER_


#include "searching.h"

#define RED 0
#define BLACK 1





typedef struct {
	EBST_STREAM * file_stream;
	
	//
	uint32_t registries_written;


	ERBT_Header header;

	frame_t frame;

} ERBT_Builder;



// Generates the file of the binary search tree + calls the balance function
bool ERBT_Build(REG_STREAM *, ERBT_STREAM *);
bool ERBT_insert(ERBT_Builder * _builder, const registry_pointer * _Entry);
void ERBT_Balance(ERBT_Builder * _builder, ebst_ptr _NodeIndex);

#define midpoint(l, r)			(l + ((r - l) >> 1))
#define rightSubfileSize(l, r)	((r - l) - ((r - l) >> 1))
bool EBST_MRT_Build(REG_STREAM *, EBST_STREAM *, frame_t *,bool ascending, uint64_t _RegistriesQtt);
bool EBST_Search(EBST_STREAM * _Stream, REG_STREAM * _InputStream, const key_t _Key, registry_t * _Target);

#endif // _ES_EBST_HEADER_