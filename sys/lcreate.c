/* lcreate.c - lcreate, newlock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate()
{
	STATWORD ps;
	int	lock;
	int 	i;

	/* disable interupts */
	disable(ps);
	/* bail out if there is not an available lock */
	if ((lock=newlock())==SYSERR) {
		/* restore interupts */
		restore(ps);
		return SYSERR;
	}
	/* set this locks state */
	locks[lock].lstate = LUSED;
	/* make this lock unlocked */
	locks[lock].llocked = UNLOCKED;
	/* reset the number of readers to 0 */
	locks[lock].lnreaders = 0;
	/* reset all the locker flags to FALSE */
	for (i=0; i<NPROC; i++) {
		locks[lock].llockers[i] = FALSE;
	}
	/* lqhead and lqtail were initialized at system startup */
	/* restore interupts */
	restore(ps);
	return lock;
}


/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index 
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lock;
	int	i;

	/* loop over all lock entries */
	for (i=0; i<NLOCKS; i++) {
		/* keep decrementing the nextlock */
		lock=nextlock--;
		/* if we reach the end, start at the beginning */
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		/* if this lock is free, flag it as used */
		if (locks[lock].lstate == LFREE) {
			locks[lock].lstate = LUSED;
			return lock;
		}
	}

	return SYSERR;
}
