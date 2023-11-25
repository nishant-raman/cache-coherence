/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include "bus.h"
using namespace std;

Cache::Cache(int s,int a,int b, ulong _id, Bus* _bus, bool _protocol)
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
   id = _id;
   protocol = _protocol;
   bus = _bus;
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
      tagMask <<= 1;
      tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
         cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{
	// If processor accesses cache
   if ((op == PR_RD) || (op == PR_WR)) {
	   currentCycle++;/*per cache global counter to maintain LRU order 
	                    among cache ways, updated on every cache access*/
	         
	   if(op == 'w') writes++;
	   else          reads++;
	   
	   cacheLine * line = findLine(addr);
	   if(line == NULL)/*miss*/
	   {
		  mem_txn++;
	      if(op == 'w') writeMisses++;
	      else readMisses++;
	
	      cacheLine *newline = fillLine(addr);
	      if(op == 'w') newline->setFlags(DIRTY);    
	
		  // Processor generated bus requests
		  if(op == 'w') {
			busrdx++;
			bus->busRdX(addr,id);
		  } else {
			bus->busRd(addr,id);
		  }
	      
	   }
	   else
	   {
	      /**since it's a hit, update LRU and update dirty flag**/
	      updateLRU(line);
	      if(op == 'w') line->setFlags(DIRTY);
	   }
		// MSI
		// if miss
		// 		if rd then busrd
		// 		if wr then budrdx
		// if hit
		// 		if rd then nothing
		// 		if wr then mod
		//
		// Dragon
		// if miss busrd
		// 		if rd and busrd returns bool
		// 			if true (other cache has block) then Sc
		// 			if false then E
		// 		if wr and busrd returns bool
		// 			if true Sm
		// 			if false M
		// if hit
		// 		if rd then nothing
		// 		if wr
		// 			if E/M then mod or nothing
		// 			else busupd (returns bool)
		// 				if true then Sm
		// 				else M
	}

	// BUS RD/X 
	// for MSI
	// check if cache has cache line with addr
	// if yes then invalidate (if modified then flush)
	if ((op == BUS_RD) || (op == BUS_RDX)) {
		cacheLine* line = findLine(addr);
		if (line) {
			if (line->getFlags() == DIRTY) {
	   			mem_txn++;
      			writeBack(addr);
				flushes++;
				bus->flush(addr, id);
			}
			invalidations++;
			line->invalidate();
		}
	}
	// for Dragon
	// BusRd
	// 		E/M -> Sc/Sm, return true
	// 		if M/Sm then flush FIXME
	// BusUpd
	// 		-> Sc
	// 		update
	// 		FIXME? assert no busupd if in m and e state
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
   if(cache[i][j].isValid()) {
      if(cache[i][j].getTag() == tag)
      {
         pos = j; 
         break; 
      }
   }
   if(pos == assoc) {
      return NULL;
   }
   else {
      return &(cache[i][pos]); 
   }
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
   line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) { 
         return &(cache[i][j]); 
      }   
   }

   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].getSeq() <= min) { 
         victim = j; 
         min = cache[i][j].getSeq();}
   } 

   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   
   if(victim->getFlags() == DIRTY) {
	  mem_txn++;
      writeBack(addr);
		// FIXME is flush needed here?
   }
      
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats()
{ 
   float missrate = (float)(readMisses + writeMisses)*100/(float)(reads + writes);
	
   printf("============ Simulation results (Cache %lu) ============\n", id);
   printf("01. number of reads:                            %lu\n", reads);
   printf("02. number of read misses:                      %lu\n", readMisses);
   printf("03. number of writes:                           %lu\n", writes);
   printf("04. number of write misses:                     %lu\n", writeMisses);
   printf("05. total miss rate:                            %.2f%%\n", missrate);
   printf("06. number of writebacks:                       %lu\n", writeBacks);
   printf("07. number of memory transactions:              %lu\n", mem_txn);
   printf("08. number of invalidations:                    %lu\n", invalidations);
   printf("09. number of flushes:                          %lu\n", flushes);
   printf("10. number of BusRdX:                           %lu\n", busrdx);
   /****print out the rest of statistics here.****/
   /****follow the ouput file format**************/
}

