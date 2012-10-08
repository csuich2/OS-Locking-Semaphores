/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <lock.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int	shouldResched = FALSE;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	/* Release any locks this process is still holding */
	shouldResched = releaseallforprocess(pid);

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	if (semaph[pptr->psem].sstate == SUSED) {
				semaph[pptr->psem].semcnt++;
			} else if (pptr->plock != -1) {
				/* If this process was waiting for a lock, we need to remove it from
				 * the waiting queue and update the max wait priority */
				int lockid = getIndexForLockDescriptor(pptr->plock);
				if (lockid != SYSERR) {
					dequeue(pid);
					updateMaxWaitPriority(pptr->plock);
					int lockid = getIndexForLockDescriptor(pptr->plock);
					updatePriorityOfProcessesHoldingLock(&locks[lockid]);
					pptr->plock = -1;
				}
			}

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	if (shouldResched)
		resched();
	restore(ps);
	return(OK);
}
