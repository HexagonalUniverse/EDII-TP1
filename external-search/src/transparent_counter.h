#ifndef TRANSPARENT_COUNTER_H
#define TRANSPARENT_COUNTER_H

#define TRANSPARENT_COUNTER	true

#include <stdint.h>

#ifdef TRANSPARENT_COUNTER
extern struct __transparent_counter_t {
	struct {
		uint64_t read;
		uint64_t write;
	} reg;

	struct {
		uint64_t read;
		uint64_t write;
	} ebst;

	struct {
		uint64_t read;
		uint64_t write;
	} erbt;

	struct {
		uint64_t read;
		uint64_t write;
	} b;

	struct {
		uint64_t read;
		uint64_t write;
	} bs;

	struct {
		uint64_t search;
		uint64_t build;
	} comparisons;

} transparent_counter;


#define cmp_eq_build(key1, key2)	((key1 == key2)	&& (++ transparent_counter.comparisons.build))
#define cmp_bg_build(key1, key2)	((key1 > key2) && (++ transparent_counter.comparisons.build))
#define cmp_ls_build(key1, key2)	((key1 < key2) && (++ transparent_counter.comparisons.build))

#define cmp_eq_search(key1, key2) ((key1 == key2) && (++ transparent_counter.comparisons.search))
#define cmp_bg_search(key1, key2) ((key1 > key2) && (++ transparent_counter.comparisons.search))
#define cmp_ls_search(key1, key2) ((key1 < key2) && (++ transparent_counter.comparisons.search))

#else

#define cmp_eq_build(key1, key2)	(key1 == key2)
#define cmp_bg_build(key1, key2)	(key1 > key2)
#define cmp_ls_build(key1, key2)	(key1 < key2)

#define cmp_eq_search(key1, key2)	(key1 == key2)
#define cmp_bg_search(key1, key2)	(key1 > key2)
#define cmp_ls_search(key1, key2)	(key1 < key2)
#endif

#endif