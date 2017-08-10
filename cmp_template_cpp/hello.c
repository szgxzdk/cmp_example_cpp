#define global

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
#include "func.h"
}
#include "barrier.h"

//the number of thread
#define NPROC 8
//a variable used for mutex lock
volatile int CountLock = 0;

int main()
{
  unsigned ProcessId = 0;
  int my_id = 0;

  //initialize a barrier, this should be done before fork (for just once)
  barrier_init(NPROC);

  //create sub-threads
  for(ProcessId = 1;ProcessId < NPROC; ProcessId++)
  {
    my_id = MTA_fork();
    //new sub-thread start execution after fork
    if(my_id != 0)
      break;
  }

  //set a barrier here to synchronize those threads
  barrier_wait(MTA_getthreadID() , NPROC);
  //start computing
  MTA_Stats(1);

  //compute kernel
  {
    //lock
    mutex_lock((int *)&CountLock);

    printf("Hello thread %d\n", MTA_getthreadID());
    fflush(stdout);
   
    //unlock
    mutex_unlock((int *)&CountLock);
  }//end of compute kernel

  //while(1) ;
  
  //waif for other threads to complete
  barrier_wait(MTA_getthreadID() , NPROC);
  //end
  MTA_Stats(0);

  while(MTA_wait() == -1)
    ;

  return 0;
}

	

	
