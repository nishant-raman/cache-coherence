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

		void busRd	(ulong, ulong);
		void busRdX (ulong, ulong);
		void flush	(ulong, ulong);
		//void busUpd (ulong addr); // TODO different return type for C flag
};

#endif
