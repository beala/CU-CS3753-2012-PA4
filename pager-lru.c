/*
 * File: pager-lru.c
 * Modified by: Alex Beal
 *              http://usrsb.in
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 *  This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int alloc_page_count = 0;

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
            }
        }
        initialized = 1;
    }

    /* TODO: Implement LRU Paging */
    fprintf(stderr, "pager-lru not yet implemented. Exiting...\n");
    exit(EXIT_FAILURE);

    /* advance time for next pageit iteration */
    tick++;
}

int find_lru_page_local(int timestamps[MAXPROCESSES][MAXPROCPAGES],
                        int proc){
    int i;
    int smallest_tick=-1;
    int lru_page=-1;
    for(i=0; i<MAXPROCPAGES; ++i){
        if(timestamps[proc][i] < smallest_tick){
            smallest_tick = timestamps[proc][i];
            lru_page = i;
        }
    }
    return lru_page;
}
