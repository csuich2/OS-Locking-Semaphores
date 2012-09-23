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

