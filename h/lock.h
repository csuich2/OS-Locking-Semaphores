/* lock.h */

#include <proc.h>
#include <q.h>

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS		50	/* number of locks, if not defined	*/
#endif

#define	READ	'\01'		/* locker type flag for a reader	*/
#define	WRITE	'\02'		/* locker type flag for a writer	*/

#define UNLOCKED	'\00'	/* lock flag for unlocked		*/
#define LOCKED_READ	'\01'	/* lock flag for reading		*/
#define LOCKED_WRITE	'\02'	/* lock flag for writing		*/

#define LFREE	'\01'		/* this lock is free			*/
#define LUSED	'\02'		/* this lock is used			*/

struct	lentry {		/* lock table entry			*/
	int	ldescriptor;	/* a unique, temporary descriptor	*/
	char	lstate;		/* the state LFREE or LUSED		*/
	char 	llocked;	/* flag indicating the type of lock	*/
	int	lnreaders;	/* number of readers locking the lock	*/
	int 	lwqhead;	/* q index of head of writer list	*/
	int	lwqtail;	/* q index of tail of writer list	*/
	int	lrqhead;	/* q index of head of reader list	*/
	int	lrqtail;	/* q index of tail of reader list	*/
	char	llockers[NPROC];/* list of procs locking this lock	*/
	int	lprio;		/* max priority among all waiting procs	*/
};
extern	struct	lentry	locks[];
extern	int	nextlock;
extern	int	nextlockdescriptor;

#define	isbadlock(l)	(l<0 || l>=NLOCKS)

int linit();
int lcreate();
int ldelete(int lockdescriptor);
int lock(int ldes, int type, int priority);
int releaseall(int numlocks, long lockdescriptors);
void releaseallforprocess(int pid);
void updateMaxWaitPriority(int ldes);
void updateLockersWithPrio(struct lentry *lptr);
void updatePriorityOfProcessesHoldingLock(struct lentry *lptr);
int getIndexForLockDescriptor(int lockdescriptor);

#endif
