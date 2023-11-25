/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

// forward declaration of cache
class Bus;

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
// TODO either add states or new flag with shared/exclusive state
enum {
   INVALID = 0,
   VALID,
   DIRTY
};

enum operations {
	PR_RD 	= 'r',
	PR_WR 	= 'w',
	BUS_RD  = 'R',
	BUS_RDX = 'X',
	BUS_UPD = 'U',
	FLUSH	= 'F',
	UPDATE	= 'u'
};

/*
enum coherence_state {
	I
	C
	Sc
	Sm
	E
	M
}
*/

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()                { tag = 0; Flags = 0; }
   ulong getTag()             { return tag; }
   ulong getFlags()           { return Flags;}
   ulong getSeq()             { return seq; }
   void setSeq(ulong Seq)     { seq = Seq;}
   void setFlags(ulong flags) {  Flags = flags;}
   void setTag(ulong a)       { tag = a; }
   void invalidate()          { tag = 0; Flags = INVALID; } //useful function
   bool isValid()             { return ((Flags) != INVALID); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   ulong mem_txn, invalidations, flushes, busrdx;

   ulong id;
   Bus*	bus;
   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)   { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag) { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int, ulong, Bus*);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM()     {return readMisses;} 
   ulong getWM()     {return writeMisses;} 
   ulong getReads()  {return reads;}       
   ulong getWrites() {return writes;}
   ulong getWB()     {return writeBacks;}
   
   void writeBack(ulong) {writeBacks++;}
   virtual void ProcAccess(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);

   virtual void BusAccess(ulong,uchar) = 0;
// status update ()

   //******///
   //add other functions to handle bus transactions///
   //******///

};

class CacheMSI : public Cache {
	public:
		CacheMSI(int s,int a,int b, ulong _id, Bus* _bus) :
			Cache(s, a, b, _id, _bus) {}

		void ProcAccess (ulong, uchar);
		void BusAccess (ulong, uchar);
};

class CacheDragon : public Cache {
	public:
		CacheDragon(int s,int a,int b, ulong _id, Bus* _bus) :
			Cache(s, a, b, _id, _bus) {}

		void ProcAccess (ulong, uchar);
		void BusAccess (ulong, uchar);
};

#endif
