/*******************************************************
                          bus.h
********************************************************/

#ifndef BUS_H
#define BUS_H

#include <vector>
#include "cache.h"

//// forward declaration of cache
//class Cache;

class Bus {
	ulong num_cache;
	std::vector<Cache*> cache_array;
	
	public:
		Bus(ulong n, Cache** _cache_array);
		
		void setCache (Cache** _cache_array);
		bool busTxn (ulong, ulong, uchar);
};

#endif
