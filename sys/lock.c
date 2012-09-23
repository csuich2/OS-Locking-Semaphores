/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

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

	/* check for a bad lock, free lock or invalid type */
	if (isbadlock(ldes1) || (lptr= &locks[ldes1])->lstate==LFREE
		|| (type != READ && type != WRITE)) {
		/* restore interupts */
		restore(ps);
		return SYSERR;
	}

	/* if this lock is acquired by a reader */
	if (lptr->llocked == LOCKED_READ) {
		/* if the priority is equal to or greater than the
 		 * highest priority writer on the queue, then this
 		 * process is allowed to continue */
		if (priority >= lastkey(lptr->lwqtail)) {
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
		/* set its blocker to this lock */
		pptr->psem = ldes1;
		/* indicate the wait return value (currently OK) */
		pptr->pwaitret = OK;
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
	} else if (lptr->llocked == LOCKED_WRITE) {
		/* this process must go on the queue because writing locks
 		 * are exclusive */
		insert(currpid, lptr->lwqhead, priority);
		/* get the current process */ 
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		/* set its blocker to this lock */
		pptr->psem = ldes1;
		/* indicate the wait return value (currently OK) */
		pptr->pwaitret = OK;
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
