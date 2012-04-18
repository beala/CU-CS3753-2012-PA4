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
struct page_stat{
    int page;
    int freq;
    int *timestamp;
};

typedef struct page_stat PageStat;

void init_cfg(PageStat cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i, j, k;
    for(i=0; i<MAXPROCESSES; i++){
    for(j=0; j<MAXPROCESSES; j++){
    for(k=0; k<MAXPROCESSES; k++){
        cfg[i][j][k].page = EMPTY;
        cfg[i][j][k].freq = -1;
        cfg[i][j][k].timestamp = NULL;
    }
    }
    }
}

/* Insert into the cfg, given the last_page, the cur_page and
 * the proc. For example, if cur_page=1, last_page=0, and proc=9,
 * then '1' will be inserted into the cfg for page zero, proc 9,
 * signalling that sometime in the past, proc 9 went from page 0
 * to page 1.
 */
void insert_cfg(
        int cur_page,
        int proc,
        int last_page,
        PageStat cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES],
        int timestamps[MAXPROCESSES][MAXPROCPAGES]){
    int i;
    PageStat *slots;
    slots = cfg[proc][last_page];

    for(i=0; i<MAXPROCPAGES; i++){
        if(slots[i].page == cur_page){
            slots[i].freq++;
            break;
        }
        if(slots[i].page == EMPTY){
            slots[i].page = cur_page;
            slots[i].freq = 1;
            slots[i].timestamp = &(timestamps[proc][i]);
            break;
        }
    }
}

/* Debug function for printing the control flow graph. */
void print_cfg(PageStat cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i, j, k;
    for(i=0; i<MAXPROCESSES; i++){
        printf("Proc %d:\n", i);
        for(j=0; j<MAXPROCESSES; j++){
            printf("Page %d: ", j);
            for(k=0; k<MAXPROCESSES; k++){
                printf("%d,%d; ", cfg[i][j][k].page, cfg[i][j][k].freq);
            }
            printf("\n");
        }
    }
}

/* Predict pages
 * Return an array of pages that are most likely to
 * be needed in 100 ticks. The returned array is
 * MAXPROCPAGES long and empty cells are -1
 */
PageStat* pred_page(int proc, int cur_pc, PageStat cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    return cfg[proc][calc_page(cur_pc+101)];
}

/* Count how many non-empty PageStat elements are in an
 * array of PageStat structs.
 */
int page_count(PageStat *guesses){
    int len=0;
    while(guesses[len].page != EMPTY && len<MAXPROCPAGES)
        len++;
    return len;
}

/* Swap two PageStat elements */
void swap(PageStat* l, PageStat* r){
    PageStat tmp=*l;
    *l=*r;
    *r=tmp;
}

/* guesses: array of PageStat.
 * Desc: Sorts the array according to biggest timestamp
 *  (most recently used).
 */
void page_sort(PageStat *guesses){
    int len = page_count(guesses);
    int i;
    int swapped = 0;
    do{
        swapped=0;
        for(i=1;i<len;i++){
            //if(guesses[i-1].freq<guesses[i].freq){
            assert(guesses[i-1].timestamp != NULL &&
                   guesses[i].timestamp != NULL);
            if(*(guesses[i-1].timestamp) < *(guesses[i].timestamp)){
                swap(guesses+(i-1), guesses+i);
                swapped = 1;
            }
        }
    }while(swapped);
}

/* Desc: Prints out an array of PageStat for debugging. */
void print_guess(PageStat* guesses){
    int i;
    for(i=0; i<MAXPROCPAGES; i++){
        if(guesses[i].page == EMPTY)
            break;
        assert(guesses[i].timestamp != NULL);
        printf("%d,%d,%d; ", guesses[i].page, guesses[i].freq, *(guesses[i].timestamp));
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
    static PageStat cfg[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

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
        insert_cfg(cur_page, proctmp, last_page, cfg, timestamps);
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
        PageStat *predictions;
        int i;
        if(!q[proctmp].active)
            continue;
        /* Guess the pages that the proc will need */
        predictions = pred_page(proctmp, q[proctmp].pc, cfg);
        page_sort(predictions);
        //print_guess(predictions);
        for(i=0; i<MAXPROCPAGES; i++){
            if(predictions[i].page == EMPTY)
                break;
            pagein(proctmp, predictions[i].page);
        }
    }

    /* advance time for next pageit iteration */
    tick++;
}
