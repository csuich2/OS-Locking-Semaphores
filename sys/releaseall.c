/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

void updateProcessPriority(int pid);
void releaselock(int pid, int lockid);

/*------------------------------------------------------------------------
 * releaseall  --  releases all specified locks for the current process 
 *------------------------------------------------------------------------
 */
int releaseall(int numlocks, int ldes1, ...)
{
	STATWORD ps;
	int i;
	int ldes;
	int retval = OK;

	/* disable interupts */
	disable(ps);
	
	/* check for a bad number of locks */
	if (numlocks < 1 || numlocks > NLOCKS) {
		/* restore interupts */
		restore(ps);
		return SYSERR;
	}

	/* initialize the variable list of arguments */
	/* loop over each lock descriptor */
	for (i=0; i<numlocks; i++) {
		/* get the next lock descriptor from the argument list */
		ldes = *(&ldes1 + i);
		/* check for a bad lock, free lock and that the calling
 		 * process has acquired this lock */
		int lockid = getIndexForLockDescriptor(ldes);
		if (isbadlock(lockid) || locks[lockid].lstate == LFREE ||
		    locks[lockid].llockers[currpid] == FALSE) {
			/* make sure we return that there was an error */
			retval = SYSERR;
			/* continue releasing the other locks */
			continue;
		}
		releaselock(currpid, lockid);
	}

	/* update the current processes priority based on its base priority and the
	 * max priority of all the processes waiting on its remainging locks */
	updateProcessPriority(currpid);

	/* reschedule the processor since the waiting queue may have changed */
	resched();

	/* restore interupts */
	restore(ps);
	return retval;
}

int releaseallforprocess(int pid)
{
	int	i;
	int	heldLock = FALSE;
	for (i=0; i<NLOCKS; i++) {
		if (locks[i].llockers[pid] == TRUE) {
			heldLock = TRUE;
			releaselock(pid, i);
		}
	}
	return heldLock;
}

void releaselock(int releasingpid, int lockid)
{
	struct	lentry *lptr;
	int	readprio;
	int	writeprio;
	int	pid;

	lptr = &locks[lockid];
	/* update the number of readers if locked by a reader */
	lptr->llockers[releasingpid] = FALSE;
	if (lptr->llocked == LOCKED_READ) {
		lptr->lnreaders--;
	}
	/* give someone else the lock if locked by a writer or by a reader and
	 * there are no more readers locking */
	if ((lptr->llocked == LOCKED_READ && lptr->lnreaders == 0) ||
	    lptr->llocked == LOCKED_WRITE) {
		/* get the max priorities for the reader and writer queues */
		readprio = lastkey(lptr->lrqtail);
		writeprio = lastkey(lptr->lwqtail);
		/* if they're both MININT, then there is nothing on the queue,
		 * so only continue if one of them is not MININT */
		if (readprio != MININT || writeprio != MININT) {
			/* only has readers, so let all readers have the lock */
			if (readprio != MININT && writeprio == MININT) {
				lptr->llocked = LOCKED_READ;
				while ((pid=getlast(lptr->lrqtail)) != EMPTY) {
					ready(pid, RESCHNO);
					proctab[pid].plock = -1;
					lptr->llockers[pid] = TRUE;
					lptr->lnreaders++;
				}
			/* let the top writer go if there are only writers waiting
			 * or if the top writer has the highest priority */
			} else if (readprio == MININT || writeprio >= readprio) {
				pid = getlast(lptr->lwqtail);
				ready(pid, RESCHNO);
				proctab[pid].plock = -1;
				lptr->llockers[pid] = TRUE;
				lptr->llocked = LOCKED_WRITE;
			/* otherwise, keep giving readers locks until the remaining
			 * readers have priority lower than the highest writer */
			} else {
				lptr->llocked = LOCKED_READ;
				while (lastkey(lptr->lrqtail) > writeprio &&
				       ((pid=getlast(lptr->lrqtail)) != EMPTY)) {
					ready(pid, RESCHNO);
					proctab[pid].plock = -1;
					lptr->llockers[pid] = TRUE;
					lptr->lnreaders++;
				}
			}
			/* enforce the process priority inheritence by making sure that all
			 * all the lockers of this lock use the new max priority of all the
			 * processes waiting for this lock */
			updateMaxWaitPriority(locks[lockid].ldescriptor);
			updatePriorityOfProcessesHoldingLock(&locks[lockid]);
		} else {
			lptr->llocked = UNLOCKED;
		}
	}
}

void updateProcessPriority(int pid)
{
	int	i;
	int	maxprio = 0;
	/* loop over all locks and */
	for (i=0; i<NLOCKS; i++) {
		/* if the lock is free or not locked by this process, skip it */
		if (locks[i].lstate == LFREE || locks[i].llockers[pid] == FALSE)
			continue;
		/* otherwise, update the running max to the max prio of the procs
		 * waiting for this lock, if it's the hightest so far */
		maxprio = max(maxprio, locks[i].lprio);
	}
	/* if no prio was found, set the inherited prio to 0 so we don't use it */
	if (maxprio == 0) {
		proctab[pid].pinh = 0;
	/* otherwise, set the inherited prio to the max of the wait listed procs and the base prio */
	} else {
		proctab[pid].pinh = max(proctab[pid].pprio, maxprio);
	}
}
