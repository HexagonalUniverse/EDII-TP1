/*	<src/include/ebst.h>

	Holds the External Binary Search Tree (EBST for short) algorithms. 
	Defines two different types of EBST's: a "pure", that can only be 
	built on ordered conditions, and the ERBT (External Red Black Tree.) */


#ifndef _ES_EBST_HEADER_
#define _ES_EBST_HEADER_


#include "searching.h"


#define RED		0		// Represents the ERBT node's coloration as "red".
#define BLACK	1		// Represents the ERBT node's coloration as "black".

// ERBT Struct
typedef struct {
	EBST_STREAM * file_stream;
	uint32_t registries_written;
	ERBT_Header header;
	Frame frame;
} ERBT_Builder;


// Generates the file of the red-black tree + calls the balance function
bool ERBT_Build(REG_STREAM *, ERBT_STREAM *);

//  Inserts a registry in the ERBT data-structure. Returns success.
bool ERBT_insert(ERBT_Builder * _builder, const registry_pointer * _Entry);

//  Balance the ERBT data-structure.
void ERBT_Balance(ERBT_Builder * _builder, ebst_ptr _NodeIndex);

//  Search a registry in the ERBT data-structure. Returns success.
bool ERBT_Search(ERBT_STREAM * _Stream, REG_STREAM * _InputStream, Frame * _Frame, const key_t _Key, registry_t * _Target);

#define midpoint(l, r)			(l + ((r - l) >> 1))
#define rightSubfileSize(l, r)	((r - l) - ((r - l) >> 1))

// Generates the file of the binary search tree + calls the balance function
bool EBST_MRT_Build(REG_STREAM *, EBST_STREAM *, Frame *,bool ascending, uint64_t _RegistriesQtt);

//  Search a registry in the EBST data-structure. Returns success.
bool EBST_Search(EBST_STREAM * _Stream, REG_STREAM * _InputStream, const key_t _Key, registry_t * _Target);

#endif // _ES_EBST_HEADER_
