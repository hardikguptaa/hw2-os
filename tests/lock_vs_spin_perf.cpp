#include <iostream>
#include <chrono>
#include <cstdlib>
#include "../lib/uthread.h"      // uthread.h is in lib/
#include "../lib/Lock.h"         // Lock.h is in lib/
#include "../lib/SpinLock.h"     // SpinLock.h is in lib/

using namespace std;
using namespace std::chrono;

#define UTHREAD_TIME_QUANTUM 10000

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
    volatile int dummy = 0;
    for (int i = 0; i < workIterations; i++) {
        dummy += i;
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
