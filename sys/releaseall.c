/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

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
	for (i=0; i<numlocks-1; i++) {
		/* get the next lock descriptor from the argument list */
		ldes = *(&lockdescriptors + i);
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
			if (readprio != MININT || writeprio != MININT) {
				/* only has readers, so let all readers have the lock */
				if (readprio != MININT && writeprio == MININT) {
					while ((pid=getlast(lptr->lrqtail)) != EMPTY) {
						ready(pid, RESCHNO);
					}
				/* let the top writer go if there are only writers waiting
 				 * or if the top reader has the highest priority */
				} else if (readprio == MININT || writeprio >= readprio) {
					ready(getlast(lptr->lwqtail), RESCHNO);
				/* otherwise, keep giving readers locks until the remaining
 				 * readers have priority lower than the highest reader */
				} else {
					while (lastkey(lptr->lrqtail) > writeprio &&
					       ((pid=getlast(lptr->lrqtail)) != EMPTY)) {
						ready(pid, RESCHNO);
					}
				}
			}
		}
	}

	/* restore interupts */
	restore(ps);
	return retval;
}
