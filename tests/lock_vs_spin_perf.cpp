/*
 * Purpose:
 *   This program evaluates and compares the performance of two locking mechanisms—
 *   a blocking Lock versus a busy-waiting SpinLock—in a user-level thread library.
 *
 *   It creates a specified number of threads, where each thread repeatedly enters a critical 
 *   section to increment a shared counter and perform a configurable amount of dummy computation 
 *   (using a square root calculation loop) while holding the lock.
 *
 *   The program accepts four command-line arguments:
 *     1. Mode: "lock" to use the blocking Lock or "spin" to use the SpinLock.
 *     2. Number of threads to create.
 *     3. Number of iterations per thread (how many times each thread will enter the critical section).
 *     4. Critical section work: the number of dummy loop iterations to simulate computational work 
 *        while the lock is held.
 *
 *   It measures the total elapsed time for all threads to complete their work and prints:
 *     - The final shared counter value (which should equal num_threads * iterations_per_thread).
 *     - The elapsed time in seconds.
 */

#include <iostream>
#include <chrono>
#include <cstdlib>
#include "../lib/uthread.h"      // uthread.h is in lib/
#include "../lib/Lock.h"         // Lock.h is in lib/
#include "../lib/SpinLock.h"     // SpinLock.h is in lib/
#include <cmath>

using namespace std;
using namespace std::chrono;

#define UTHREAD_TIME_QUANTUM 100000

// Global shared counter used in the test.
static long long sharedCounter = 0;

// Global synchronization objects.
static Lock perfLock;
static SpinLock perfSpinLock;

// Global test parameters (set via command-line arguments)
int globalIterations = 0;  // Number of iterations per thread
int globalCsWork = 0;      // Dummy work iterations inside critical section
bool useLock = true;       // true: use Lock; false: use SpinLock

// Dummy function to simulate work inside the critical section.
void perform_work(int workIterations) {
    //volatile int dummy = 0;

    volatile double dummy = 1.0;
    for (int i = 0; i < workIterations; i++) {
        dummy = sqrt(dummy * i + 1.0);
    }
}

// Thread function for the performance test.
void* performanceTestFunc(void* arg) {
    for (int i = 0; i < globalIterations; i++) {
        if (useLock) {
            perfLock.lock();
            sharedCounter++;
            perform_work(globalCsWork);
            perfLock.unlock();
        } else {
            perfSpinLock.lock();
            sharedCounter++;
            perform_work(globalCsWork);
            perfSpinLock.unlock();
        }
        if (i % 1000 == 0) {
            uthread_yield();
        }
    }
    return nullptr;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <lock|spin> <num_threads> <iterations_per_thread> <cs_work>" << endl;
        return 1;
    }
    string mode = argv[1];
    int numThreads = atoi(argv[2]);
    globalIterations = atoi(argv[3]);
    globalCsWork = atoi(argv[4]);

    if (mode == "lock")
        useLock = true;
    else if (mode == "spin")
        useLock = false;
    else {
        cerr << "Invalid mode: " << mode << ". Use 'lock' or 'spin'." << endl;
        return 1;
    }

    if (uthread_init(UTHREAD_TIME_QUANTUM) != 0) {
        cerr << "Error: uthread_init failed." << endl;
        return 1;
    }
    sharedCounter = 0;
    int* tids = new int[numThreads];

    auto start_time = high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        tids[i] = uthread_create(performanceTestFunc, nullptr);
    }
    for (int i = 0; i < numThreads; i++) {
        uthread_join(tids[i], nullptr);
    }
    auto end_time = high_resolution_clock::now();
    double elapsed_time_seconds = duration_cast<duration<double>>(end_time - start_time).count();

    cout << "Performance Evaluation - " << (useLock ? "Lock" : "SpinLock") << " Test" << endl;
    cout << "Number of threads       : " << numThreads << endl;
    cout << "Iterations per thread   : " << globalIterations << endl;
    cout << "Critical section work   : " << globalCsWork << " iterations" << endl;
    cout << "Final sharedCounter     : " << sharedCounter << endl;
    cout << "Elapsed time            : " << elapsed_time_seconds << " seconds" << endl;

    delete [] tids;
    return 0;
}
