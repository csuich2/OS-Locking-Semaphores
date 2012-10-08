/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

void reinsertIfNecessary(int pid, struct pentry *pptr);

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	int	lockid;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if (pptr->pprio >= pptr->pinh) {
		pptr->pinh = 0;
	}
	if (pptr->plock != -1 && (lockid=getIndexForLockDescriptor(pptr->plock))!=SYSERR) {
		updateMaxWaitPriority(pptr->plock);
		updatePriorityOfProcessesHoldingLock(&locks[lockid]);
	}
	reinsertIfNecessary(pid, pptr);
	restore(ps);

	return(newprio);
}

void reinsertIfNecessary(int pid, struct pentry *pptr)
{
	if(pptr -> pstate == PRREADY) {
		dequeue(pid);
		insert(pid, rdyhead, max(pptr->pprio, pptr->pinh));
	}
}

void updatePriorityOfProcessesHoldingLock(struct lentry *lptr)
{
	int	i, j;
	struct	pentry	*pptr;
	int	lockid;

	for (i=0; i<NPROC; i++) {
		if (lptr->llockers[i] == FALSE)
			continue;

		pptr = &proctab[i];

		int maxprio = 0;
		for (j=0; j<NLOCKS; j++) {
			if (locks[j].llockers[i] == TRUE)
				maxprio = max(maxprio, locks[j].lprio);
		}
		if (maxprio > pptr->pprio)
			pptr->pinh = maxprio;
		else
			pptr->pinh = 0;
		reinsertIfNecessary(i, pptr);

		if (pptr->plock != -1 && (lockid=getIndexForLockDescriptor(pptr->plock))!=SYSERR) {
			updateMaxWaitPriority(pptr->plock);
			updatePriorityOfProcessesHoldingLock(&locks[lockid]);
		}
	}
}
