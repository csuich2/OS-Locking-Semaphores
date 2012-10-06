/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void halt();
int main1();
int writer1(char*, int, int);
int writer2(char*, int, int);
int writer3(char*, int, int);

int w1, w2, w3;
int lck1, lck2, lck3;

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	main1();
	//main2();
	return 0;
}

int main1()
{
        int i;
        int j = 0;
	kprintf("in main1\n");
        /*create lock*/
        lck1 = lcreate();
	lck2 = lcreate();
	lck3 = lcreate();

        kprintf("\n\nTEST1:\n");
        resume(w1 = create(writer1,2000,20,"reader1",3,"writer1", lck1, 30));
        resume(w2 = create(writer2,2000,20,"reader2",3,"writer2", lck1, 30));
        resume(w3 = create(writer3,2000,20,"writer1",3,"writer3", lck1, 30));

        sleep(3);
        kill(w2);
        kprintf ("Test 1 finished!\n");

	return 1;
}

int writer1(char* name, int lck, int prio){
    kprintf ("  %s: acquiring lock 1\n", name);
    lock (lck, WRITE, prio);
    kprintf ("  %s: acquired lock 1, sleep 4s\n", name);
    sleep (10);
    kprintf ("  %s: to release lock 1\n", name);
    releaseall (1, lck);
};

int writer2(char* name, int lck, int prio){
    kprintf ("  %s: acquiring lock 2\n", name);
    lock (lck2, WRITE, prio);
    kprintf ("  %s: acquired lock 2\n", name);
    kprintf ("  %s: acquaring lock 1\n", name);
    lock (lck1, WRITE, prio);
    kprintf ("  %s: acquired lock 1\n", name);
    sleep (4);
    kprintf ("  %s: to release lock\n", name);
    releaseall (1, lck);
};

int writer3(char* name, int lck, int prio){
    kprintf ("  %s: acquiring lock 3\n", name);
    lock (lck3, WRITE, prio);
    kprintf ("  %s: acquired lock 3\n", name);
    kprintf ("  %s: acquiring lock 2\n", name);
    lock (lck2, WRITE, prio);
    kprintf ("  %s: acquired lock 2\n", name);
    sleep (4);
    kprintf ("  %s: to release lock\n", name);
    releaseall (1, lck);
};
