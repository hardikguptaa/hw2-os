/*
 * Purpose:
 *   This program tests the correctness and performance of the SpinLock implementation
 *   in the user-level thread library. It creates a fixed number of threads (NUM_THREADS),
 *   and each thread performs a loop where it increments a shared counter ITERATIONS times.
 *
 *   The critical section (the shared counter increment) is protected by a SpinLock to ensure
 *   mutual exclusion. In addition, each thread yields control every 1000 iterations using
 *   uthread_yield() to force context switches and simulate real concurrency.
 */
 
#include <iostream>
#include "../lib/uthread.h"      // Headers in lib/
#include "../lib/SpinLock.h"

using namespace std;

#define NUM_THREADS 5
#define ITERATIONS 10000

static int sharedCounter = 0;
static SpinLock spinlock;

void* spinlockTestFunc(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        spinlock.lock();
        sharedCounter++;
        spinlock.unlock();
        if (i % 1000 == 0)
            uthread_yield();
    
    }
    return nullptr;
}

int main() {
    if (uthread_init(10000) != 0) {
        cerr << "Error: uthread_init failed!" << endl;
        return 1;
    }
    int tids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        tids[i] = uthread_create(spinlockTestFunc, nullptr);
        cout << "Created thread with id " << tids[i] << endl;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(tids[i], nullptr);
    }
    cout << "Final sharedCounter = " << sharedCounter 
         << " (expected " << NUM_THREADS * ITERATIONS << ")" << endl;
    return 0;
}