#include <stdio.h>

extern "C" {
#include "func.h"
}
#include "barrier.h"

#ifdef BAR_OPTI
volatile int global_tree_flag;
#endif

struct tree_node tree[MAXTHREADS];
volatile int local_sense[MAXTHREADS];
volatile int tree_level;
volatile int Partial_tree;
int barcnt;

void TreeLevel(int P)
{
	int i = 0;
	for(i=0;i<11;i++)
	{ /* try to find the multiple of 2, and P can not exceed 1024*/
		if(((1<<i) & P) == (1<<i))
			break;
	}
	printf("barrier: tree level = %d\n", i);
	if(i%2 == 0) /* P is the not the multiple of 4, which means this is not a completed tree */
	{
		Partial_tree = 0;
		tree_level = i/2;
	}
	else 
	{
		Partial_tree = 1;
		tree_level = i/2 + 1;
	}
	if(tree_level == 0)
	{
		Partial_tree = 1;
		tree_level = 1; /* make the tree level at least is 1 for P = 1*/
	}
	return;
}

void barrier_init(int P)
{
	int i=0, j=0, threadid, m=0, NodeNum;
	barcnt = 0;

	TreeLevel(P);
#ifdef BAR_OPTI
	global_tree_flag = 1;
	MTA_Bar_Stats(&global_tree_flag);
#endif
	for(barcnt = 0; barcnt < P; barcnt++)
		local_sense[barcnt] = 0;
	for(threadid=0;threadid<P;threadid++)
	{
		for(i=0;i<TREE_BRANCH*tree_level;i++)
		{
			tree[threadid].child_exists[i] = 1;
			tree[threadid].child_ready[i] = 1;
#ifndef BAR_OPTI
			tree[threadid].wkup_child_flags[i] = 1;
#endif
		}
		for(j=0;j<tree_level;j++)
		{
			if((Partial_tree) && (j == tree_level - 1) && threadid == P-1 && P != 1)
			{ /* the top level of the tree */
				for(i=TREE_BRANCH*j;i<(TREE_BRANCH*(j+1));i++)
					/* Partial tree is for the branch is not equal to 4, e.g. P = 2 and P = 8*/
					if(i%4 == 0)
					{
						tree[threadid].child_exists[i] = 0;
						tree[threadid].child_ready[i] = 0;
					}
			}
			NodeNum = 1;
			for(m=0;m<(j+1);m++)
				NodeNum = NodeNum * TREE_BRANCH;	
			if(((threadid+1)%(NodeNum)) == 0)
			{ /* Node of the tree */
				for(i=TREE_BRANCH*j;i<(TREE_BRANCH*(j+1));i++)
				{ /* Initialize the level flag */	
					if((i)%4 != 3)
					{
						tree[threadid].child_exists[i] = 0;
						tree[threadid].child_ready[i] = 0;
					}
				}
			}
			else if(P == 2 && threadid == 1)
			{
				tree[threadid].child_exists[0] = 0;
				tree[threadid].child_ready[0] = 0;
			}
		}
	}
	for(barcnt = 0; barcnt < P; barcnt++)
	{
		printf("barrier: threadid %d:", barcnt);
		for(i=0; i<tree_level*TREE_BRANCH; i++)
			printf("%d ", tree[barcnt].child_exists[i]);
		printf("\n");
	}	
}

/* thread synchronization barrier */
void barrier(int threadid, int P) // FIXME: combining tree that spins on local variables only
{
	int i, j, n, m, busy_wait_counter=0, node=0, entry=-1, type = tree_level, NodeNum = 1;
	asm("wmb");	/* alpha consistency memory barrier */

	if(threadid != P-1)
	{
		if(P == 2)
		{
			if(threadid == 0)	
			{
				type = 0;
				node = 2;
				entry = threadid%TREE_BRANCH;
			}
		}
		else if((threadid+1)%TREE_BRANCH != 0)
		{ /* leaves of the tree */
			type = 0;
			node = TREE_BRANCH*(1+threadid/TREE_BRANCH);
			entry = threadid%TREE_BRANCH;
		}
		else
		{ /* nodes of the tree */
			for(j=1;j<tree_level;j++)
			{
				NodeNum = 1;
				for(m=0;m<(j+1);m++)
					NodeNum = NodeNum * TREE_BRANCH;	
				type = j;
				if(Partial_tree && (j == tree_level-1))
				{
					if(((threadid+1)%(NodeNum)) == (NodeNum/TREE_BRANCH))
					{ /* leaves of the tree */
						node = P;
						entry = (threadid+1)/(NodeNum/TREE_BRANCH) - 1 + TREE_BRANCH*j;
						break;
					}
				}
				else
				{
					if(((threadid+1)%(NodeNum)) != 0)
					{ /* leaves of the tree */
						node = NodeNum*(1+threadid/NodeNum);
						entry = ((threadid+1)%NodeNum)/(NodeNum/TREE_BRANCH) - 1 + TREE_BRANCH*j;
						break;
					}
				}
			}
		}
	}
	else
		type = tree_level;

	for(i=0;i<TREE_BRANCH*type;i++)  /* Busy wait for node*/
		while(!tree[threadid].child_ready[i]) {MTA_acquiesce(&(tree[threadid].child_ready[i]));};


	for(i=0;i<TREE_BRANCH*type;i++)  /* reset the child ready bits with child exists for next barrier using */
		tree[threadid].child_ready[i] = tree[threadid].child_exists[i];

	if(threadid != P-1)
	{
		tree[node-1].child_ready[entry] = 1; /* set the parent's child_ready bits */
#ifdef BAR_OPTI
		while(global_tree_flag != local_sense[threadid]){MTA_acquiesce(&(global_tree_flag));}; /* wait for the local sense */
#else
		while(tree[node-1].wkup_child_flags[entry] != local_sense[threadid]){MTA_acquiesce(&(tree[node-1].wkup_child_flags[entry]));}; /* wait for the local sense */
#endif
	}

	if(threadid == P-1)
		MTA_Bar_Stats(2); /* indicated that update the tree is finished */
	/* release the barrier */
#ifdef BAR_OPTI
	if(threadid == P-1)
		global_tree_flag = local_sense[threadid];
#else
	for(i=0;i<TREE_BRANCH*tree_level;i++)
		tree[threadid].wkup_child_flags[i] = local_sense[threadid];
#endif
	local_sense[threadid] = !local_sense[threadid]; /* reverse the local sense for next barrier */

	if(threadid == P-1)
		MTA_Bar_Instr();
	asm("mb");
}
