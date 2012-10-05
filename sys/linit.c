/* linit.c - linit */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <lock.h>

/*------------------------------------------------------------------------
 *  linit  --  initialize all Xinu locks 
 *------------------------------------------------------------------------
 */
int linit() 
{
	int i;
	struct lentry *lptr;

	for (i=0; i<NLOCKS; i++) {	/* for each lock */
		(lptr = &locks[i])->lstate = LFREE;
		lptr->lwqtail = 1 + (lptr->lwqhead = newqueue());
		lptr->lrqtail = 1 + (lptr->lrqhead = newqueue());
	}

	return OK;
}

void updateMaxWaitPriority(int ldes)
{
	struct	lentry *lptr;
	int	next;
	int lockid = getIndexForLockDescriptor(ldes);
	lptr = &locks[lockid];
	int maxprio = 0;
	next = q[lptr->lrqhead].qnext;
	while (q[next].qnext != EMPTY) {
		int procprio = max(proctab[next].pprio, proctab[next].pinh);
		maxprio = max(maxprio, procprio);
		next = q[next].qnext;
	}
	next = q[lptr->lwqhead].qnext;
	while (q[next].qnext != EMPTY) {
		int procprio = max(proctab[next].pprio, proctab[next].pinh);
		maxprio = max(maxprio, procprio);
		next = q[next].qnext;
	}
	lptr->lprio = maxprio;
}

int getIndexForLockDescriptor(int ldes) {
	int	i;
	for (i=0; i<NLOCKS; i++) {
		if (locks[i].ldescriptor == ldes)
			return i;
	}
	return SYSERR;
}
