/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <math.h>

void updateMaxWaitPrio(struct pentry *pptr, struct lentry *lptr);
void updateProcessForWaiting(struct pentry *pptr, int ldes);


/*------------------------------------------------------------------------
 * lock  --  make current process wait on a lock 
 *------------------------------------------------------------------------
 */
int lock(int ldes1, int type, int priority)
{
	STATWORD ps;
	struct	lentry	*lptr;
	struct	pentry	*pptr;

	/* disable interupts */
	disable(ps);

	int lockid = getIndexForLockDescriptor(ldes1);
	/* check for a bad lock, free lock or invalid type */
	if (isbadlock(lockid) || (lptr= &locks[lockid])->lstate==LFREE
		|| (type != READ && type != WRITE)) {
		/* restore interupts */
		restore(ps);
		return SYSERR;
	}

	/* if this lock is acquired and being requested by a reader */
	if (lptr->llocked != UNLOCKED && type == READ) {
		/* if the lock is held by a reader and if the
		 * request priority is equal to or greater than the
 		 * highest priority writer on the queue, then this
 		 * process is allowed to continue */
		if (lptr->llocked == LOCKED_READ &&
		    priority >= lastkey(lptr->lwqtail)) {
			/* bump the number of readers */
			lptr->lnreaders++;
			/* set this processes locker flag to TRUE */
			lptr->llockers[currpid] = TRUE;
			/* restore interupts */
			return OK;
		}
		/* otherwise, since its priority is lower, it should
 		 * go on the reader queue */
		insert(currpid, lptr->lrqhead, priority);
		/* get the current process */ 
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		/* update this locks max wait prio based on this new waiter */
		updateMaxWaitPrio(pptr, lptr);
		/* update the pentry for waiting */
		updateProcessForWaiting(pptr, ldes1);
		/* reschedule since this process is now blocked */
		/* a context switch should happen here */
		resched();
		/* when we get here, this reader has the lock now, so: */
		/* bump the number of readers */
		lptr->lnreaders++;
		/* set this processes locker flag to TRUE */
		lptr->llockers[currpid] = TRUE;
		/* restore interupts */
		restore(ps);
		/* return the wait return value */
		return pptr->pwaitret;
	/* else if this lock is ackquired by a writer */
	} else if (lptr->llocked != UNLOCKED && type == WRITE) {
		/* this process must go on the queue because writing locks
 		 * are exclusive */
		insert(currpid, lptr->lwqhead, priority);
		/* get the current process */ 
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		/* update the pentry for waiting */
		updateProcessForWaiting(pptr, ldes1);
		/* update this locks max wait prio based on this new waiter */
		updateMaxWaitPrio(pptr, lptr);
		/* reschedule since this process is now blocked */
		/* a context switch should happen here */
		resched();
		/* when we get here, this writer has the lock now, so: */
		/* set this processes locker flag to TRUE */
		lptr->llockers[currpid] = TRUE;
		/* restore interupts */
		restore(ps);
		return pptr->pwaitret;
	}

	/* otherwise, the lock is free, so acquire it and set
 	 * the flag and number of readers accordingly */
	if (type == READ) {
		lptr->llocked = LOCKED_READ;
		lptr->lnreaders = 1;
	} else {
		lptr->llocked = LOCKED_WRITE;
		lptr->lnreaders = 0;
	}
	/* set this processes locker flag to TRUE */
	lptr->llockers[currpid] = TRUE;

	/* restore interupts */
	restore(ps);
	return OK;
}

/* pptr should be a pentry for the current process
 * lptr should be a lentry for the lock currently being worked on
 */
void updateMaxWaitPrio(struct pentry *pptr, struct lentry *lptr)
{
	/* get the correct priority from pptr to use
	 * which is pprio, unless pinh is set (not 0) */
	int currpidprio = max(pptr->pprio, pptr->pinh);
	/* if the soon to be waiting process' priority is higher
	 * than all the other waiting processes, update the lock
	 * with the new highest wait priority and then ramp up
	 * the priority of all processes holding this lock */
	if (lptr->lprio < currpidprio) {
		lptr->lprio = currpidprio;
		//propogateInheritedPriority(pptr, currpidprio);
		updatePriorityOfProcessesHoldingLock(lptr);
	}
}

/* pptr should be a pentry for the current process
 * ldes should be the lock descriptor for the plock field of pptr
 */
void updateProcessForWaiting(struct pentry *pptr, int ldes)
{
	/* set its blocker to this lock */
	pptr->plock = ldes;
	/* indicate the wait return value (currently OK) */
	pptr->pwaitret = OK;
}
