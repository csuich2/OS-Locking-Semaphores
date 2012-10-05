/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void halt();

int main2();
int looper(int);
int waiter(int);
int waiter2(int);

int reader1, reader2;
int writer1;
int lck1;

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	main2();
	return 0;
}

int main2()
{
	int lck = lcreate();
	int loopProc, waitProc, waitProc2;
	resume(loopProc = create(looper,2000,20,"looper",1,lck));
	resume(waitProc = create(waiter,2000,50,"waiter",1,lck));
	resume(waitProc2 = create(waiter2,2000,40,"waiter",1,lck));
	sleep(1);
	chprio(waitProc, 30);
}

int looper(int lck)
{
	int i;
	lock(lck, WRITE, 100);
	for (i=0; i<5; i++) {
		sleep(1);
		kprintf("looper pprio: %d\n", proctab[currpid].pprio);
		kprintf("looper pinh: %d\n", proctab[currpid].pinh);
	}
	releaseall(1, lck);
}

int waiter(int lck)
{
	int i;
	lock(lck, WRITE, 100);
	releaseall(1, lck);
}

int waiter2(int lck)
{
	int i;
	lock(lck, WRITE, 50);
	kprintf("waiter2 got the lock\n");
	releaseall(1, lck);
}
