/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry 
 *------------------------------------------------------------------------
 */
int ldelete(int lockdescriptor)
{
	STATWORD ps;
	int	pid;
	struct	lentry	*lptr;

	/* disable interupts */
	disable(ps);

	/* check for a bad lock or free lock */
	int lockid = getIndexForLockDescriptor(lockdescriptor);
	if (isbadlock(lockid) || locks[lockid].lstate==LFREE) {
		/* restore interupts */
		restore(ps);
		return SYSERR;
	}
	/* get the lock entry */
	lptr = &locks[lockid];
	/* set its state to free */
	lptr->lstate = LFREE;
	/* set its locked flag to unlocked */
	lptr->llocked = UNLOCKED;
	/* if there are waiting processes, awake them */
	if (nonempty(lptr->lrqhead) || nonempty(lptr->lwqhead)) {
		/* loop over all processes in the reader queue */
		while ((pid=getfirst(lptr->lrqhead)) != EMPTY) {
			/* update the wait return to deleted */
			proctab[pid].pwaitret = DELETED;
			proctab[pid].plock = -1;
			/* put the process back on the ready queue */
			ready(pid, RESCHNO);
		}
		/* same as above, but for the writer queue */
		while ((pid=getfirst(lptr->lwqhead)) != EMPTY) {
			proctab[pid].pwaitret = DELETED;
			proctab[pid].plock = -1;
			ready(pid, RESCHNO);
		}
		/* reschedule with newly readied processes */
		resched();
	}
	/* restore interupts */
	restore(ps);
	return OK;
}
