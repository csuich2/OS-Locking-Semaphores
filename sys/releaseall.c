/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <math.h>

void updateProcessPriority(int currpid);

/*------------------------------------------------------------------------
 * releaseall  --  releases all specified locks for the current process 
 *------------------------------------------------------------------------
 */
int releaseall(int numlocks, long lockdescriptors)
{
	STATWORD ps;
	register struct lentry *lptr;
	int i;
	int ldes;
	int retval = OK;
	int readprio;
	int writeprio;
	int pid;

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
	//kprintf2("%d released %d locks\n", currpid, numlocks);
	for (i=0; i<numlocks; i++) {
		/* get the next lock descriptor from the argument list */
		ldes = *(&lockdescriptors + i);
		//kprintf2("releasing %d\n", ldes);
		/* check for a bad lock, free lock and that the calling
 		 * process has acquired this lock */
		if (isbadlock(ldes) || locks[ldes].lstate == LFREE ||
		    locks[ldes].llockers[currpid] == FALSE) {
			/* make sure we return that there was an error */
			retval = SYSERR;
			/* continue releasing the other locks */
			continue;
		}
		lptr = &locks[ldes];
		/* update the number of readers if locked by a reader */
		lptr->llockers[currpid] = FALSE;
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
			//kprintf2("choosing who to give lock %d to\n", ldes);
			if (readprio != MININT || writeprio != MININT) {
				/* only has readers, so let all readers have the lock */
				if (readprio != MININT && writeprio == MININT) {
					lptr->llocked = LOCKED_READ;
					while ((pid=getlast(lptr->lrqtail)) != EMPTY) {
						ready(pid, RESCHNO);
						//kprintf2("lock given to %d\n", pid);
						proctab[pid].plock = -1;
						lptr->llockers[pid] = TRUE;
						lptr->lnreaders++;
					}
				/* let the top writer go if there are only writers waiting
				 * or if the top writer has the highest priority */
				} else if (readprio == MININT || writeprio >= readprio) {
					pid = getlast(lptr->lwqtail);
					//kprintf2("lock given to %d\n", pid);
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
						//kprintf2("lock given to %d\n", pid);
						proctab[pid].plock = -1;
						lptr->llockers[pid] = TRUE;
						lptr->lnreaders++;
					}
				}
				/* enforce the process priority inheritence by making sure that all
				 * all the lockers of this lock use the new max priority of all the
				 * processes waiting for this lock */
				updateMaxWaitPriority(ldes);
				//kprintf2("max wait prio set to: %d\n", locks[ldes].lprio);
				updatePriorityOfProcessesHoldingLock(&locks[ldes]);
			} else {
				//kprintf2("no one in queue to receive lock\n");
				lptr->llocked = UNLOCKED;
			}
		}
	}

	/* update the current processes priority based on its base priority and the
	 * max priority of all the processes waiting on its remainging locks */
	updateProcessPriority(currpid);

	/* restore interupts */
	restore(ps);
	return retval;
}

void updateProcessPriority(int currpid)
{
	int	i;
	int	maxprio = 0;
	/* loop over all locks and */
	for (i=0; i<NLOCKS; i++) {
		/* if the lock is free or not locked by this process, skip it */
		if (locks[i].lstate == LFREE || locks[i].llockers[currpid] == FALSE)
			continue;
		/* otherwise, update the running max to the max prio of the procs
		 * waiting for this lock, if it's the hightest so far */
		maxprio = max(maxprio, locks[i].lprio);
	}
	/* if no prio was found, set the inherited prio to 0 so we don't use it */
	if (maxprio == 0) {
		proctab[currpid].pinh = 0;
	/* otherwise, set the inherited prio to the max of the wait listed procs and the base prio */
	} else {
		proctab[currpid].pinh = max(proctab[currpid].pprio, maxprio);
	}
}
