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

enum last_call {
    IN,
    OUT,
    NONE
};

int pagein_better(int proc, int page, int *proc_page_count, enum last_call status[MAXPROCESSES][MAXPROCPAGES], Pentry q[MAXPROCESSES]){
    /* It's already swapped in */
    if(q[proc].pages[page] == 1){
        return 2;
    }
    /* It's already started swapping in */
    if(status[proc][page] == IN){
        return 3;
    }
    /* Ok, try to swap in. If it works, update
     * status, etc */
    if(pagein(proc,page)){
        status[proc][page]=IN;
        proc_page_count[proc]++;
        return 1;
    }
    return 0;
}

int pageout_better(int proc, int page, int proc_page_count[MAXPROCESSES],
                   enum last_call status[MAXPROCESSES][MAXPROCPAGES],
                   Pentry q[MAXPROCESSES]){
    /* It's already swapped out, or is in progress */
    if(status[proc][page] == OUT || q[proc].pages[page] == -1)
        return 2;
    /* Ok, try to swap out. If it works, update
     * status, etc */
    if(pageout(proc,page)){
        printf("Swapping out proc %d page %d\n", proc, page);
        status[proc][page]=OUT;
        proc_page_count[proc]--;
        return 1;
    }
    return 0;
}

int find_lru_page_local(int timestamps[MAXPROCESSES][MAXPROCPAGES],
                        int proc,
                        Pentry q[MAXPROCESSES]){
    int i;
    int smallest_tick=INT_MAX;
    int lru_page=-1;
    for(i=0; i<MAXPROCPAGES; ++i){
        if(q[proc].pages[i] == 1 && timestamps[proc][i] < smallest_tick){
            smallest_tick = timestamps[proc][i];
            lru_page = i;
        }
    }
    return lru_page;
}

void pageit(Pentry q[MAXPROCESSES]) { 

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int proc_page_count[MAXPROCESSES];
    static int per_proc = 5;
    /*  1 if the most recent call was successful swapin.
     * -1 if the most recent call was successful swapout.
     *  0 intial value.*/
    static enum last_call swapstat[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int lru_page;

    /* initialize static vars on first run */
    if(!initialized){
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
                swapstat[proctmp][pagetmp] = NONE;
            }
            proc_page_count[proctmp] = 0;
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
                if(!pageout_better(proctmp, pagetmp, proc_page_count, swapstat, q)){
                    fprintf(stderr,"ERROR: Proc dead, but can't swap out page!\n");
                    exit(EXIT_FAILURE);
                }
            }
            continue;
        }
        /* Calc the next page the proc will need */
        pagetmp = (q[proctmp].pc)/PAGESIZE;
        /* If it's swapped in, skip */
        if(q[proctmp].pages[pagetmp] == 1)
            continue;
        /* If there are pages available, swap it in
         * and move onto next proc*/
        if(proc_page_count[proctmp] < per_proc){
            if(!pagein_better(proctmp, pagetmp, proc_page_count, swapstat, q)){
                fprintf(stderr, "ERROR: Page in failed even though there should be empty pages!\n");
                exit(EXIT_FAILURE);
            }
            continue;
        }
        /* Proc needs page, but all frames are taken.
         * Get the LRU page, swap it out, and swap in the
         * needed page */
        lru_page = find_lru_page_local(timestamps, proctmp, q);
        if(lru_page < 0){
            fprintf(stderr, "ERROR: Cound't find page to evict!\n");
            int i;
            //for(i=0; i<MAXPROCPAGES; i++){
            //    fprintf(stderr, "Page %d. Stat: %ld. Alloc: %d. Proc: %d\n",
            //            i, q[proctmp].pages[i], proc_page_count[proctmp], proctmp);
            //    int j;
            //    for(j=0; j<MAXPROCPAGES; j++)
            //        fprintf(stderr, "%d ", swapstat[proctmp][j]);
            //    fprintf(stderr,"\n");
            //}
            continue;
            exit(EXIT_FAILURE);
        }
        //printf("LRU Page: Prog %d Page %d\nAlloc'd: %d\n", proctmp, lru_page, proc_page_count[proctmp]);
        if(!pageout_better(proctmp, lru_page, proc_page_count, swapstat, q)){
            fprintf(stderr, "ERROR: Paging out LRU page failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    /* advance time for next pageit iteration */
    tick++;
}
