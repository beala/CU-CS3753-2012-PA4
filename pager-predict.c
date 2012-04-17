/*
 * File: pager-predict.c
 * Modified by: Alex Beal
 *              http://usrsb.in
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 *  This file contains an predict pageit
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

int calc_page(int pc){
    return pc/PAGESIZE;
}

#define EMPTY -1

void init_cfg(int cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i, j, k;
    for(i=0; i<MAXPROCESSES; i++){
    for(j=0; j<MAXPROCESSES; j++){
    for(k=0; k<MAXPROCESSES; k++){
        cfg[i][j][k] = EMPTY;
    }
    }
    }
}

void insert_cfg(int cur_page, int proc, int last_page, int cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i;
    int *slots;
    slots = cfg[proc][last_page];

    for(i=0; i<MAXPROCPAGES; i++){
        if(slots[i] == cur_page){
            break;
        }
        if(slots[i] == EMPTY){
            slots[i] = cur_page;
            break;
        }
    }
}

void print_cfg(int cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i, j, k;
    for(i=0; i<MAXPROCESSES; i++){
        printf("Proc %d:\n", i);
    for(j=0; j<MAXPROCESSES; j++){
        printf("Page %d: ", j);
    for(k=0; k<MAXPROCESSES; k++){
        printf("%d, ", cfg[i][j][k]);
    }
        printf("\n");
    }
    }
}

int* pred_page(int proc, int cur_pc, int cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    return cfg[proc][calc_page(cur_pc+100)];
}

void print_guess(int* guesses){
    int i;
    for(i=0; i<MAXPROCPAGES; i++){
        if(guesses[i] == EMPTY)
            break;
        printf("%d, ", guesses[i]);
    }
    printf("\n");
}

void pageit(Pentry q[MAXPROCESSES]) { 

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int proc_stat[MAXPROCESSES];
    static int pc_last[MAXPROCESSES];

    /* Each process has a 20 item long array, with each element
     * pointing to another 20 element array */
    static int cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int lru_page;
    int last_page;
    int cur_page;

    /* initialize static vars on first run */
    if(!initialized){
        init_cfg(cfg);
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
            }
            proc_stat[proctmp] = 0;
        }
        initialized = 1;
    }

    /* Update control flow graph */
    for(proctmp=0; proctmp<MAXPROCESSES; proctmp++){
        /* Skip if inactive */
        if(!q[proctmp].active)
            continue;
        /* Skip if the last last page is -1 */
        if(last_page == -1)
            continue;
        last_page = calc_page(pc_last[proctmp]);
        /* Update pc_last */
        pc_last[proctmp] = q[proctmp].pc;
        /* Skip if last is same as current */
        cur_page = calc_page(q[proctmp].pc);
        if(last_page == cur_page)
            continue;
        /* While we're at it, page out the last page */
        pageout(proctmp, last_page);
        insert_cfg(cur_page, proctmp, last_page, cfg);
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

    for(proctmp=0; proctmp<MAXPROCESSES; proctmp++){
        int *predictions;
        int i;
        if(!q[proctmp].active)
            continue;
        /* Guess the pages that the proc will need */
        predictions = pred_page(proctmp, q[proctmp].pc, cfg);
        for(i=0; i<MAXPROCPAGES; i++){
            if(predictions[i] == EMPTY)
                break;
            pagein(proctmp, predictions[i]);
        }
    }

    /* advance time for next pageit iteration */
    tick++;
}
