
// ==============================================
// Includes
// ==============================================

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <proc.h>

// ==============================================
// Function primitives
// ==============================================

void test_basic_rw_locking();
void test_basic_lock_priority();
void test_basic_priority_inheritance();
void test_complex_prio_inheritance();
void test_basic_lock_deletion();
void test_rw_and_inheritance();
void test_multiple_lock_inheritance();
void test_multiple_lock_inheritance_and_kill();

int reader(int sleeplen, int lck, int lockprio);
int writer(int sleeplen, int lck, int lockprio);
int doublewriter(int sleeplen, int lck1, int lck2, int lockprio);
int twowriter(int sleep1len, int sleep2len, int lck1, int lck2, int lockprio);
int readerdeleter(int sleeplen, int lck, int lockprio);

int getcurrenteffectiveprio();
char* getcurrentname();

// ==============================================
// Main
// ==============================================

int main(){

	kprintf("\n\n");
	//test_basic_rw_locking();
	//test_basic_lock_priority();
	//test_basic_priority_inheritance();
	//test_complex_prio_inheritance();
	//test_basic_lock_deletion();
	//test_rw_and_inheritance();
	//test_multiple_lock_inheritance();
	test_multiple_lock_inheritance_and_kill();

	return 0;
	
}

// ==============================================
// Test functions
// ==============================================

// PURPOSE: Tests basic read and write lock functionality
// BEHAVIOR: R1,R2 should immediately get the lock, while W3 waits.
void test_basic_rw_locking(){

	// Function variables
	int reader1, reader2, writer3;

	// Create 1 lock
	int lck1 = lcreate();
	
	// Print expected output
	kprintf("test_basic_rw_locking expected: R1(20)-w R1(20)-h R2(20)-w R2(20)-h W3(20)-w R2(20)-r R1(20)-r W3(20)-h W3(20)-r\n");
	kprintf("test_basic_rw_locking actual  : ");
	
	// Start processes
	resume(reader1 = create(reader, 2000, 20, "R1", 3, 4, lck1, 20));
	resume(reader2 = create(reader, 2000, 20, "R2", 3, 2, lck1, 20));
	resume(writer3 = create(writer, 2000, 20, "W3", 3, 1, lck1, 20));
	
	// Wait until we can be sure processes are finished
	sleep(10);
	
	// Clean up
	kill(reader1);
	kill(reader2);
	kill(writer3);
	ldelete(lck1);
	kprintf("\n\n");
	
}

// PURPOSE: Tests lock priority functionality. 
// BEHAVIOR: R1 should immediately get the lock, W2 should wait, and W3 should hop W2 in the line to get the lock first.
void test_basic_lock_priority(){

	// Function variables
	int reader1, writer2, writer3;

	// Create 1 lock
	int lck1 = lcreate();
	
	// Print expected output
	kprintf("test_basic_lock_priority expected: R1(20)-w R1(20)-h W2(20)-w W3(20)-w R1(20)-r W3(20)-h W3(20)-r W2(20)-h W2(20)-r\n");
	kprintf("test_basic_lock_priority actual  : ");
	
	// Start processes
	resume(reader1 = create(reader, 2000, 20, "R1", 3, 3, lck1, 20));
	resume(writer2 = create(writer, 2000, 20, "W2", 3, 1, lck1, 20));
	resume(writer3 = create(writer, 2000, 20, "W3", 3, 1, lck1, 30));
	
	// Wait until we can be sure processes are finished
	sleep(10);
	
	// Clean up
	kill(reader1);
	kill(writer2);
	kill(writer3);
	ldelete(lck1);
	kprintf("\n\n");
	
}

// PURPOSE: Tests basic lock priority inheritance for 1 lock.
// BEHAVIOR: Same as test_basic_lock_priority but R1,W3 should see their effective priority bumped up due to W2, who has high process priority but low lock priority
void test_basic_priority_inheritance(){

	// Function variables
	int reader1, writer2, writer3;

	// Create 1 lock
	int lck1 = lcreate();
	
	// Print expected output
	kprintf("test_basic_priority_inheritance expected: R1(20)-w R1(20)-h W2(50)-w W3(20)-w R1(50)-r W3(50)-h W3(50)-r W2(50)-h W2(50)-r\n");
	kprintf("test_basic_priority_inheritance actual  : ");
	
	// Start processes
	resume(reader1 = create(reader, 2000, 20, "R1", 3, 3, lck1, 20));
	resume(writer2 = create(writer, 2000, 50, "W2", 3, 1, lck1, 20));
	resume(writer3 = create(writer, 2000, 20, "W3", 3, 1, lck1, 30));
	
	// Wait until we can be sure processes are finished
	sleep(10);
	
	// Clean up
	kill(reader1);
	kill(writer2);
	kill(writer3);
	ldelete(lck1);
	kprintf("\n\n");
	
}

// PURPOSE: Tests complex lock priority inheritance across three locks
// BEHAVIOR: Three locks are created. A hold/wait chain is formed and a high-priority waiter on the first lock should propogate to the holder of the third lock.
void test_complex_prio_inheritance(){

	// Function variables
	int doublewriter1, doublewriter2, writer3, writer4;

	// Create 3 lock
	int lck1 = lcreate();
	int lck2 = lcreate();
	int lck3 = lcreate();
	
	// Print expected output
	kprintf("test_complex_prio_inheritance expected: DW1(20)-w#1 DW1(20)-h#1 DW2(20)-w#1 DW2(20)-h#1 W3(20)-w W3(20)-h DW1(20)-w#2 DW2(20)-w#2 W4(70)-w W3(70)-r DW2(70)-h#2 DW2(70)-r#1-#2 DW1(70)-h#2 DW1(70)-r#1-#2 W4(70)-h W4(70)-r\n");
	kprintf("test_complex_prio_inheritance actual  : ");
	
	// Start processes
	resume(doublewriter1 = create(doublewriter, 2000, 20, "DW1", 4, 5, lck1, lck2, 20));
	resume(doublewriter2 = create(doublewriter, 2000, 20, "DW2", 4, 3, lck2, lck3, 20));
	resume(writer3 = create(writer, 2000, 20, "W3", 3, 10, lck3, 20));
	sleep(5);
	resume(writer4 = create(writer, 2000, 70, "W4", 3, 1, lck1, 20));
	
	// Wait until we can be sure processes are finished
	sleep(30);
	
	// Clean up
	kill(doublewriter1);
	kill(doublewriter2);
	kill(writer3);
	kill(writer4);
	ldelete(lck1);
	ldelete(lck2);
	ldelete(lck3);
	kprintf("\n\n");

}

// PURPOSE: Tests lock deletion functionality
// BEHAVIOR: RD1 gets the lock while W2 waits. RD1 deletes the lock. W2 should be awoken with the deleted constant.
void test_basic_lock_deletion(){

	// Function variables
	int readerdeleter1, writer2;

	// Create 1 lock
	int lck1 = lcreate();
	
	// Print expected output
	kprintf("test_basic_lock_deletion expected: RD1(20)-w RD1(20)-h W2(20)-w RD1(20)-d W2(20)-el(%d)\n", DELETED);
	kprintf("test_basic_lock_deletion actual  : ");
	
	// Start processes
	resume(readerdeleter1 = create(readerdeleter, 2000, 20, "RD1", 3, 2, lck1, 20));
	resume(writer2 = create(writer, 2000, 20, "W2", 3, 1, lck1, 20));
	
	// Wait until we can be sure processes are finished
	sleep(6);
	
	// Clean up
	kill(readerdeleter1);
	kill(writer2);
	kprintf("\n\n");
	
}

void test_rw_and_inheritance() {
	
	// Function variables
	int reader1, reader2, reader3;
	int writer1, writer2;

	// Create 1 lock
	int lck1 = lcreate();

	// Print expected output
	kprintf("test_rw_and_inheritance expected: W1(20)-w W1(20)-h R1(30)-w R2(35)-w W2(20)-w R3(50)-w W1(50)-r R1(50)-h R2(50)-h R1(50)-r R2(50)-r W2(50)-h W2(50)-r R3(50)-h R3(50)-r\n");
	kprintf("test_rw_and_inheritance actual  : ");

	// Start processes
	resume(writer1 = create(writer, 2000, 20, "W1", 3, 3, lck1, 50));
	resume(reader1 = create(reader, 2000, 30, "R1", 3, 1, lck1, 40));
	resume(reader2 = create(reader, 2000, 35, "R2", 3, 2, lck1, 30));
	resume(writer2 = create(writer, 2000, 20, "W2", 3, 1, lck1, 20));
	resume(reader3 = create(reader, 2000, 50, "R3", 3, 1, lck1, 10));

	// Wait until we can be sure processes are finished
	sleep(15);
	
	// Clean up
	kill(writer1);
	kill(reader1);
	kill(reader2);
	kill(writer2);
	kill(reader3);
	ldelete(lck1);
	kprintf("\n\n");
}

void test_multiple_lock_inheritance() {

	// Function variables
	int multwriter;
	int writer1, writer2;
	int reader1, reader2;

	// Create 2 locks
	int lck1 = lcreate();
	int lck2 = lcreate();

	// Print expected output
	kprintf("test_multiple_lock_inheritance expected: TW1(20)-w#1 TW1(20)-h#1 TW1(20)-w#2 TW1(20)-h#2 W1(30)-w R1(40)-w R2(50)-w W2(60)-w TW1(60) TW1(40) TW1(40)-r#1-#2 W1(40)-h R2(35)-h W1(40)-r R1(40)-h R2(35)-r W2(35)-h R1(40)-r W2(35)-R\n");
	kprintf("test_multiple_lock_inheritance actual  : ");

	// Start processes
	resume(multwriter = create(twowriter, 2000, 20, "TW1", 5, 2, 2, lck1, lck2, 60));
	resume(writer1   = create(writer,    2000, 30,  "W1", 3, 1, lck1, 50));
	resume(reader1   = create(reader,    2000, 40,  "R1", 3, 1, lck1, 40));
	resume(reader2   = create(reader,    2000, 50,  "R2", 3, 1, lck2, 50));
	resume(writer2   = create(writer,    2000, 60,  "W2", 3, 1, lck2, 40));
	
	// Wait for the two writer to wake up and print
	sleep(2);

	// Change reader2 and writer2's prio so twowriter inherits from reader1
	chprio(reader2, 25);
	chprio(writer2, 35);

	// Wait until we can be sure processes are finished
	sleep(7);

	// Clean up
	kill(multwriter);
	kill(writer1);
	kill(reader1);
	kill(reader2);
	kill(writer2);
	ldelete(lck1);
	ldelete(lck2);
	kprintf("\n\n");
}

void test_multiple_lock_inheritance_and_kill() {

	// Function variables
	int multwriter;
	int writer1, writer2;
	int reader1, reader2;

	// Create 2 locks
	int lck1 = lcreate();
	int lck2 = lcreate();

	// Print expected output
	kprintf("test_multiple_lock_inheritance expected: TW1(20)-w#1 TW1(20)-h#1 TW1(20)-w#2 TW1(20)-h#2 W1(30)-w R1(50)-w R2(40)-w W2(60)-w TW1(50) TW1(40) TW1(40)-r#1-#2 W1(30)-h R2(40)-h W1(30)-r R2(40)-r\n");
	kprintf("test_multiple_lock_inheritance actual  : ");

	// Start processes
	resume(multwriter = create(twowriter, 2000, 20, "TW1", 5, 2, 2, lck1, lck2, 60));
	resume(writer1   = create(writer,    2000, 30,  "W1", 3, 1, lck1, 50));
	resume(reader1   = create(reader,    2000, 50,  "R1", 3, 1, lck1, 40));
	resume(reader2   = create(reader,    2000, 40,  "R2", 3, 1, lck2, 50));
	resume(writer2   = create(writer,    2000, 60,  "W2", 3, 1, lck2, 40));

	// Kill writer2 so multwriter inherits prio from reader1
	kill(writer2);
	
	// Wait for the two writer to wake up and print
	sleep(2);

	// Kill reader1 so multwriter inherits prio from reader2
	kill(reader1);

	// Wait until we can be sure processes are finished
	sleep(7);

	// Clean up
	kill(multwriter);
	kill(writer1);
	kill(reader1);
	kill(reader2);
	kill(writer2);
	ldelete(lck1);
	ldelete(lck2);
	kprintf("\n\n");
}

// ==============================================
// Process functions
// ==============================================

// Holds a lock for reading
int reader(int sleeplen, int lck, int lockprio){
	kprintf("%s(%d)-w ", getcurrentname(), getcurrenteffectiveprio());
	int lockret = lock(lck, READ, lockprio);
	if(lockret == OK){
		kprintf("%s(%d)-h ", getcurrentname(), getcurrenteffectiveprio());
		sleep(sleeplen);
		kprintf("%s(%d)-r ", getcurrentname(), getcurrenteffectiveprio());
		int releaseret = releaseall(1, lck);
		if(releaseret != OK)
			kprintf("%s(%d)-er(%d) ", getcurrentname(), getcurrenteffectiveprio(), releaseret);
	}
	else
		kprintf("%s(%d)-el(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
}

// Holds a lock for writing
int writer(int sleeplen, int lck, int lockprio){
   	kprintf("%s(%d)-w ", getcurrentname(), getcurrenteffectiveprio());
	int lockret = lock(lck, WRITE, lockprio);
	if(lockret == OK){
		kprintf("%s(%d)-h ", getcurrentname(), getcurrenteffectiveprio());
		sleep(sleeplen);
		kprintf("%s(%d)-r ", getcurrentname(), getcurrenteffectiveprio());
		int releaseret = releaseall(1, lck);
		if(releaseret != OK)
			kprintf("%s(%d)-er(%d) ", getcurrentname(), getcurrenteffectiveprio(), releaseret);
	}
	else
		kprintf("%s(%d)-el(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
}

// Holds two locks for writing. Sleeps in between the acquires to allow creation of hold/wait chains
int doublewriter(int sleeplen, int lck1, int lck2, int lockprio){
	kprintf("%s(%d)-w#1 ", getcurrentname(), getcurrenteffectiveprio());
	int lockret = lock(lck1, WRITE, lockprio);
	if(lockret == OK){
		kprintf("%s(%d)-h#1 ", getcurrentname(), getcurrenteffectiveprio());
		sleep(2);
		kprintf("%s(%d)-w#2 ", getcurrentname(), getcurrenteffectiveprio());
		lockret = lock(lck2, WRITE, lockprio);
		if(lockret == OK){
			kprintf("%s(%d)-h#2 ", getcurrentname(), getcurrenteffectiveprio());			
			sleep(sleeplen);			
			kprintf("%s(%d)-r#1-#2 ", getcurrentname(), getcurrenteffectiveprio());
			releaseall(2, lck1, lck2);
		}
		else
			kprintf("%s(%d)-e#2(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
	}
	else
		kprintf("%s(%d)-e#1(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
}

int twowriter(int sleep1len, int sleep2len, int lck1, int lck2, int lockprio) {
	kprintf("%s(%d)-w#1 ", getcurrentname(), getcurrenteffectiveprio());
	int lockret = lock(lck1, WRITE, lockprio);
	if (lockret == SYSERR) {
		kprintf("%s(%d)-e#1(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
	} else {
		kprintf("%s(%d)-h#1 ", getcurrentname(), getcurrenteffectiveprio());
		kprintf("%s(%d)-w#2 ", getcurrentname(), getcurrenteffectiveprio());
		lockret = lock(lck2, WRITE, lockprio);
		if (lockret == SYSERR) {
			kprintf("%s(%d)-e#2(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
		} else {
			kprintf("%s(%d)-h#2 ", getcurrentname(), getcurrenteffectiveprio());
			sleep(sleep1len);
			kprintf("%s(%d) ", getcurrentname(), getcurrenteffectiveprio());
			sleep(sleep2len);
			kprintf("%s(%d) ", getcurrentname(), getcurrenteffectiveprio());
			kprintf("%s(%d)-r#1-#2 ", getcurrentname(), getcurrenteffectiveprio());
			releaseall(2, lck1, lck2);
		}
	}
}

// Holds a lock for reading and then deletes it
int readerdeleter(int sleeplen, int lck, int lockprio){
	kprintf("%s(%d)-w ", getcurrentname(), getcurrenteffectiveprio());
	int lockret = lock(lck, READ, lockprio);
	if(lockret == OK){
		kprintf("%s(%d)-h ", getcurrentname(), getcurrenteffectiveprio());
		sleep(sleeplen);
		kprintf("%s(%d)-d ", getcurrentname(), getcurrenteffectiveprio());
		int deleteret = ldelete(lck);
		if(deleteret != OK)
			kprintf("%s(%d)-ed(%d) ", getcurrentname(), getcurrenteffectiveprio(), deleteret);
	}
	else
		kprintf("%s(%d)-el(%d) ", getcurrentname(), getcurrenteffectiveprio(), lockret);
}

// ==============================================
// Utility functions
// ==============================================

// Gets the current effective priority
int getcurrenteffectiveprio(){

	// Use pinh if it exists
	if(proctab[currpid].pinh > 0){		
		return proctab[currpid].pinh;		
	}
	// Otherwise use prio
	else{		
		return proctab[currpid].pprio;		
	}

}

// Gets the current process name
char* getcurrentname(){
	return proctab[currpid].pname;
}
