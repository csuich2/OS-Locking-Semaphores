/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void halt();

int main2();
int looper(int);
int waiter(int, int, int);

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
	int lck2 = lcreate();
	int lck3 = lcreate();
	int loopProc, waitProc, waitProc2;
	resume(loopProc = create(looper,2000,20,"looper",1,lck));
	resume(waitProc = create(waiter,2000,50,"waiter",3,lck,lck2,lck3));
	sleep(1);
	kprintf("looper inh prio before kill: %d\n", proctab[loopProc].pinh);
	kill(waitProc);
	kprintf("looper inh prio after kill: %d\n", proctab[loopProc].pinh);
}

int looper(int lck)
{
	int i;
	int origIndex = getIndexForLockDescriptor(lck);
	int origDes = lck;
	lock(lck, WRITE, 100);
	sleep(5);
}

int waiter(int lck, int lck2, int lck3)
{
	int i;
	lock(lck2, WRITE, 100);
	lock(lck3, WRITE, 100);
	lock(lck, WRITE, 100);
	kprintf("waiter shouldn't get here!!!!\n");
}
