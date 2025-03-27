/*
 * Purpose:
 *   This program demonstrates the combined use of Lock and CondVar to implement a 
 *   producer–consumer scenario with a bounded buffer.
 *
 *   Producers produce a fixed number of items (set by PRODUCER_ITEMS) and insert them
 *   into a circular buffer. Consumers remove items from the buffer. The shared buffer
 *   is protected by a Lock to ensure mutual exclusion, while two condition variables,
 *   notFull and notEmpty, are used to coordinate waiting and signaling between producers
 *   and consumers.
 *   This test case verifies that the Lock and CondVar functions work correctly to prevent race
 *   conditions, ensure mutual exclusion, and coordinate thread blocking and waking in a producer–consumer model.
 */
 
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define BUFFER_SIZE 5
#define PRODUCER_ITEMS 3  // Each producer will produce this many items

// Shared buffer
static int buffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int count_items = 0;

// Lock + CondVars
static Lock bufferLock;
static CondVar notFull;   // signaled/broadcast when an item is removed or producers finished
static CondVar notEmpty;  // signaled/broadcast when an item is added or we want consumers to exit

// Global flag to indicate that no more items will be produced.
static bool producers_done = false;
// Global flag to indicate that consumers should exit if the buffer is empty.
static bool consumers_should_stop = false;

// Simple checks
void assertBufferInvariants() {
    assert(count_items >= 0 && count_items <= BUFFER_SIZE);
    // Additional checks if desired
}

// Producer thread function
void* producer(void* arg) {
    int producerID = *(int*)arg;
    int itemsLeft = PRODUCER_ITEMS;

    while (itemsLeft > 0) {
        bufferLock.lock();
        // Wait if buffer is full
        while (count_items == BUFFER_SIZE) {
            notFull.wait(bufferLock);  // release lock, block, reacquire on signal
        }
        // Insert an item
        assertBufferInvariants();
        buffer[head] = producerID;  
        head = (head + 1) % BUFFER_SIZE;
        count_items++;
        itemsLeft--;

        cout << "[Producer " << producerID << "] Produced one item (left=" 
             << itemsLeft << ")." << endl;

        // Signal one consumer that there's an item
        notEmpty.signal();
        bufferLock.unlock();

        // Optional yield
        if (rand() % 100 < 30) {
            uthread_yield();
        }
    }

    cout << "[Producer " << producerID << "] Done producing all items!" << endl;
    return nullptr;
}

// Consumer thread function
void* consumer(void* arg) {
    int consumerID = *(int*)arg;

    while (true) {
        bufferLock.lock();

        // If buffer is empty, wait unless producers are all done 
        // and there's nothing left to consume => we might exit
        while (count_items == 0 && !consumers_should_stop) {
            notEmpty.wait(bufferLock); 
        }

        // If we were signaled/broadcast because producers are finished 
        // and we should stop, check the global flags:
        if (count_items == 0 && consumers_should_stop) {
            // Nothing left to consume => break out
            bufferLock.unlock();
            break;
        }

        // Otherwise, remove one item
        assertBufferInvariants();
        int item = buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        count_items--;

        cout << "[Consumer " << consumerID << "] Consumed item from Producer "
             << item << endl;

        // Signal that space is freed up
        notFull.signal();
        bufferLock.unlock();

        // Optional yield
        if (rand() % 100 < 30) {
            uthread_yield();
        }
    }

    cout << "[Consumer " << consumerID << "] Exiting (no more items)!" << endl;
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <num_producers> <num_consumers>" << endl;
        return 1;
    }
    int numProducers = atoi(argv[1]);
    int numConsumers = atoi(argv[2]);

    if (uthread_init(UTHREAD_TIME_QUANTUM) < 0) {
        cerr << "Error: uthread_init failed." << endl;
        return 1;
    }

    // Create producers
    int* producerIDs = new int[numProducers];
    int producerTids[100]; // store TIDs so we can join
    for (int i = 0; i < numProducers; i++) {
        producerIDs[i] = i + 1;
        producerTids[i] = uthread_create(producer, &producerIDs[i]);
    }

    // Create consumers
    int* consumerIDs = new int[numConsumers];
    int consumerTids[100];
    for (int i = 0; i < numConsumers; i++) {
        consumerIDs[i] = i + 1;
        consumerTids[i] = uthread_create(consumer, &consumerIDs[i]);
    }

    // Join all producers so we know they finished 
    // producing all their items.
    for (int i = 0; i < numProducers; i++) {
        uthread_join(producerTids[i], nullptr);
    }
    cout << "[Main] All producers have finished producing." << endl;

    // We now set global flags and do a broadcast 
    // to wake up any waiting consumers so they can exit if no items remain
    bufferLock.lock();
    producers_done = true;
    consumers_should_stop = true; 
    // broadcast so that any consumer stuck in notEmpty.wait() will wake
    notEmpty.broadcast();
    bufferLock.unlock();

    // Now join consumers so they can exit gracefully
    for (int i = 0; i < numConsumers; i++) {
        uthread_join(consumerTids[i], nullptr);
    }

    cout << "[Main] All consumers have exited." << endl;

    delete[] producerIDs;
    delete[] consumerIDs;
    return 0;
}
