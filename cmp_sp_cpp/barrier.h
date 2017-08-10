#ifndef _BARRIER_H_
#define _BARRIER_H_

extern "C" {
#include "func.h"
}

/* Combining tree barrier */
/* Each node contains two arrays, one is for child arriving flag (child_ready[]), and the other is the child spin flag (wkup_child_flags[]);
   The radix of the tree is 4, if the total number of thread is 16, then it will be two levels. */

//#define BAR_OPTI

#define TREE_BRANCH	4
#define MAXTHREADS	256			//Number of node that we can simulate at most 
#define MAXTREELEVEL	4

struct tree_node {
#ifndef BAR_OPTI
	volatile int wkup_child_flags[TREE_BRANCH*MAXTREELEVEL];  /* flags for children in wake-up tree */
#endif
	volatile int child_ready[TREE_BRANCH*MAXTREELEVEL];  /*flags for children in arrival tree */
	volatile int child_exists[TREE_BRANCH*MAXTREELEVEL]; 
};

void barrier_init(int P);
void barrier(int threadid, int P); // FIXME: combining tree that spins on local variables only

inline void barrier_wait(int tid, int tnum) {
  MTA_Bar_Stats(1);
  barrier(tid, tnum);
  MTA_Bar_Stats(0);
}

inline void mutex_lock(int * lock) {
  MTA_Lock_Stats(1);
  MTA_Lock_Acquire(lock);
  MTA_Lock_Stats(0);
}

inline void mutex_unlock(int * lock) {
  MTA_Lock_Stats(1);
  MTA_Lock_Release(lock);
  MTA_Lock_Stats(0);
}


#endif
