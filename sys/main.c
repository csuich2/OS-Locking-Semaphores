/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void halt();
int main1();
int reader(char*, int, int);
int writer(char*, int, int);

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
	//main1();
	main2();
	return 0;
}

int main2()
{
	int lck = lcreate();
	int loopProc, waitProc, waitProc2;
	resume(loopProc = create(looper,2000,20,"looper",1,lck));
	resume(waitProc = create(waiter,2000,50,"waiter",1,lck));
	sleep(1);
	resume(waitProc2 = create(waiter2,2000,40,"waiter",1,lck));
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
	kprintf("lock released by looper\n");
	sleep(1);
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
	kprintf("waiter got the lock\n");
	sleep(2);
	kprintf2("waiter inherited priority: %d\n", proctab[currpid].pinh);
	releaseall(1, lck);
}

int waiter2(int lck)
{
	int i;
	lock(lck, WRITE, 60);
	kprintf("waiter2 got the lock\n");
	releaseall(1, lck);
}

/* test1.c
 * This test program creates three processes, two readers and a writer at
 * priority 20.  The main process also has priority 20.
 * All readers and writers have the same lock priority.
 */

int main1()
{
        int i;
        int j = 0;
	kprintf("in main1\n");
        /*create lock*/
        lck1 = lcreate();


        kprintf("\n\nTEST1:\n");
        resume(reader1 = create(reader,2000,20,"reader1",3,"reader1", lck1, 30));
        resume(reader2 = create(reader,2000,20,"reader2",3,"reader2", lck1, 30));
        resume(writer1 = create(writer,2000,20,"writer1",3,"writer1", lck1, 30));

        sleep(10);
        ldelete (lck1);
        kprintf ("Test 1 finished!\n");

	return 1;
}


int reader(char* name, int lck, int prio){
    kprintf ("  %s: acquiring lock\n", name);
    lock (lck, READ, prio);
    kprintf ("  %s: acquired lock, sleep 2s\n", name);
    sleep (2);
    kprintf ("  %s: to release lock\n", name);
    releaseall (1, lck);
    
    sleep(1);
    
    kprintf ("  %s: acquiring lock\n", name);
    lock (lck, READ, prio);

    kprintf ("  %s: acquired lock, sleep 2s\n", name);
    sleep (2);
    kprintf("	%s to release lock\n", name);
    releaseall (1, lck);
}

int writer(char* name, int lck, int prio){
    kprintf ("  %s: acquiring lock\n", name);
    lock (lck, WRITE, prio);
    kprintf ("  %s: acquired lock, sleep 4s\n", name);
    sleep (4);
    kprintf ("  %s: to release lock\n", name);
    releaseall (1, lck);
};
