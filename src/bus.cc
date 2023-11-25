/*******************************************************
                          bus.cc
********************************************************/

#include "cache.h"
#include "bus.h"

using namespace std;

Bus::Bus (ulong n, Cache** _cache_array)
	: num_cache(n) {
	cache_array.resize(n);
	for (ulong i=0; i<n; i++) {
		cache_array[i] = _cache_array[i];
	}
}

void Bus::setCache (Cache** _cache_array) {
	for (ulong i=0; i<num_cache; i++)
		cache_array[i] = _cache_array[i];
}

void Bus::busTxn (ulong addr, ulong pid, uchar op) {
	for (ulong i=0; i<num_cache; i++) {
		if (i != pid)
			cache_array[i]->BusAccess(addr, op);
	}
}

