#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <stdint.h>

// It is virtually impossible to get more than
// one solution per stream hash calculation
// Leave room for up to 4 results. A power
// of 2 here will yield better CUDA optimization
#define SEARCH_RESULTS 4

typedef struct {
	uint32_t count;
	struct {
		// One word for gid and 8 for mix hash
		uint32_t gid;
		uint32_t mix[8];
		uint32_t pad[7]; // pad to size power of 2
	} result[SEARCH_RESULTS];
} search_results;

#define ACCESSES 64
#define THREADS_PER_HASH (128 / 16)

typedef struct
{
	uint4 uint4s[32 / sizeof(uint4)];
} hash32_t;

typedef struct
{
	uint4	 uint4s[128 / sizeof(uint4)];
} hash128_t;
