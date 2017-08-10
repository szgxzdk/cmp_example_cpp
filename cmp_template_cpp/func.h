#ifndef _FUNC_H_
#define _FUNC_H_

int MTA_fork();
void MTA_Bar_Stats(int n);
void MTA_Stats(int n);

void MTA_Lock_Stats(int n);
void MTA_Lock_Acquire(volatile int * lock);
void MTA_Lock_Release(volatile int * lock);

void MTA_acquiesce(volatile int * status);
void MTA_Bar_Instr();

int MTA_getthreadID();

int MTA_wait();

#endif
