/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"
#include "bus.h"

int main(int argc, char *argv[])
{
    
    ifstream fin;
    FILE * pFile;

    if(argv[1] == NULL){
         printf("input format: ");
         printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
         exit(0);
        }

    ulong cache_size     = atoi(argv[1]);
    ulong cache_assoc    = atoi(argv[2]);
    ulong blk_size       = atoi(argv[3]);
    ulong num_processors = atoi(argv[4]);
    ulong protocol       = atoi(argv[5]); /* 0:MODIFIED_MSI 1:DRAGON*/
    char *fname        = (char *) malloc(20);
    fname              = argv[6];

    printf("===== Simulator configuration =====\n");
    // print out simulator configuration here
    
    // Using pointers so that we can use inheritance */
    Cache** cacheArray = (Cache **) malloc(num_processors * sizeof(Cache));
	Bus* bus = new Bus(num_processors, cacheArray);
    for(ulong i = 0; i < num_processors; i++) {
        if(protocol == 0) {
            cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size, i, bus);
        }
    }
	bus->setCache(cacheArray);


    pFile = fopen (fname,"r");
    if(pFile == 0)
    {   
        printf("Trace file problem\n");
        exit(0);
    }
    
    ulong proc;
    char op;
    ulong addr;

    int line = 1;
    while(fscanf(pFile, "%lu %c %lx", &proc, &op, &addr) != EOF)
    {
#ifdef _DEBUG
        printf("%d\n", line);
#endif
        cacheArray[proc]->Access(addr,op);
        line++;
    }

    fclose(pFile);

    //********************************//
    //print out all caches' statistics //
    //********************************//
    printf("===== 506 Personal information =====\n");
	printf("Nishant Raman\n");
	printf("nraman2\n");
	printf("ECE492 Students? NO\n");
	printf("===== 506 SMP Simulator configuration =====\n");
	printf("L1_SIZE: 		%lu\n",cache_size);
	printf("L1_ASSOC: 		%lu\n",cache_assoc);
	printf("L1_BLOCKSIZE:		%lu\n",blk_size);
	printf("NUMBER OF PROCESSORS: 	%lu\n",num_processors);
	if (protocol)
		printf("COHERENCE PROTOCOL: 	Dragon\n");
	else
		printf("COHERENCE PROTOCOL: 	MSI\n");
	printf("TRACE FILE: 		%s\n",fname);
	for (ulong i=0; i<num_processors; i++) {
		cacheArray[i]->printStats();
	}

}
