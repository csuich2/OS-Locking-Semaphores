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
}

int looper(int lck)
{
	int i;
	int origIndex = getIndexForLockDescriptor(lck);
	int origDes = lck;
	lock(lck, WRITE, 100);
	sleep(1);
	releaseall(1, lck);
	do {
		ldelete(lck);
		lck = lcreate();
	} while (getIndexForLockDescriptor(lck) != origIndex);
	kprintf("looper: orig descriptor, index: %d, %d\n", origDes, origIndex);
	kprintf("looper: new descriptor, index: %d, %d\n", lck, getIndexForLockDescriptor(lck));
}

int waiter(int lck)
{
	int i;
	sleep(5);
	int ret = lock(lck, WRITE, 100);
	kprintf("waiter: ret (%d) should equal SYSERR (%d), which it does? %d\n", ret, SYSERR, ret == SYSERR);
	ret = getIndexForLockDescriptor(lck);
	kprintf("waiter: should get SYSERR for index of deleted descriptor, and does? %d\n", ret == SYSERR); 
}
