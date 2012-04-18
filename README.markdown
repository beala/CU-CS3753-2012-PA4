###Description###
An implementation of the LRU paging algorithm, and a predictive paging algorithm for the memory manager simulator (`simulator.c`).

The predictive pager builds a control flow graph as the program runs, based on the value of the PC. It then uses this information to guess the pages that will be needed in the future, and attempts to page them in.

###Compilation###
```
make
```

###Usage###
Run LRU pager:

```
./test-lru
```

Run predictive pager:

```
./test-predict
```

###Credits###
See README.old.
