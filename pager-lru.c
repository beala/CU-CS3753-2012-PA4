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
#include <limits.h>
#include <assert.h>

#include "simulator.h"

int find_lru_page_global(int timestamps[MAXPROCESSES][MAXPROCPAGES],
                        Pentry q[MAXPROCESSES],
                        int *lru_proc,
                        int *lru_page){
    int proci, pagei;
    int smallest_tick=INT_MAX;
    int rc = 1;
    for(proci=0; proci<MAXPROCESSES; proci++){
        for(pagei=0; pagei<MAXPROCPAGES; pagei++){
            if(q[proci].pages[pagei] == 1 &&
               timestamps[proci][pagei] < smallest_tick){
                smallest_tick = timestamps[proci][proci];
                *lru_page = pagei;
                *lru_proc = proci;
                rc = 0;
            }
        }
    }
    return rc;
}

int find_lru_page_local(int timestamps[MAXPROCESSES][MAXPROCPAGES],
                        Pentry q[MAXPROCESSES],
                        int proc,
                        int *lru_page){
    int pagei;
    int smallest_tick=INT_MAX;
    int rc = 1;
    for(pagei=0; pagei<MAXPROCPAGES; pagei++){
        if(q[proc].pages[pagei] == 1 && timestamps[proc][pagei] < smallest_tick){
            *lru_page = pagei;
            smallest_tick = timestamps[proc][pagei];
            rc = 0;
        }
    }
    return rc;
}

void pageit(Pentry q[MAXPROCESSES]) { 

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int proc_stat[MAXPROCESSES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int lru_page;

    /* initialize static vars on first run */
    if(!initialized){
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
            }
            proc_stat[proctmp] = 0;
        }
        initialized = 1;
    }

    /* Update all the timestamps for active procs */
    for(proctmp=0; proctmp<MAXPROCESSES; proctmp++){
        if(!q[proctmp].active)
            continue;
        pagetmp = (q[proctmp].pc-1)/PAGESIZE;
        timestamps[proctmp][pagetmp] = tick;
    }

    for(proctmp=0; proctmp<MAXPROCESSES; proctmp++){
        /* Not active? Swap out pages and skip. */
        if (!q[proctmp].active){
            for(pagetmp=0; pagetmp<MAXPROCPAGES; pagetmp++){
                pageout(proctmp,pagetmp);
            }
            continue;
        }
        /* Calc the next page the proc will need */
        pagetmp = (q[proctmp].pc)/PAGESIZE;
        /* If it's swapped in, skip */
        if(q[proctmp].pages[pagetmp] == 1)
            continue;
        /* Try to swap in. If it works, continue */
        if(pagein(proctmp,pagetmp)){
            /* Proc is not longer waiting on a pageout */
            proc_stat[proctmp]=0;
            continue;
        }
        /* Don't pageout if a pageout for this
         * proc is already in progress. */
        if(proc_stat[proctmp])
            continue;

        /* Proc needs page, but all frames are taken.
         * Get the LRU page, swap it out, and swap in the
         * needed page */
        if(find_lru_page_local(timestamps, q, proctmp, &lru_page)){
            /* Couldn't find a page to evict. Just wait until some other
             * proc frees some pages. */
            continue;
        }
        if(!pageout(proctmp, lru_page)){
            fprintf(stderr, "ERROR: Paging out LRU page failed!\n");
            exit(EXIT_FAILURE);
        }
        /* Signal that this proc is waiting on a pageout */
        proc_stat[proctmp]=1;
    }

    /* advance time for next pageit iteration */
    tick++;
}
