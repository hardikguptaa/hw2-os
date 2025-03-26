#include "CondVar.h"
#include "uthread_private.h"
#include <cassert>

CondVar::CondVar()
{
    // Nothing to do in the constructor.
}

// Atomically release the lock and block the current thread.
// When signaled (or broadcasted), the thread will be added to the ready queue.
void CondVar::wait(Lock &lock)
{
    disableInterrupts();
    // Unlock the associated lock.
    lock._unlock();
    // Add the running thread to this condition variable's wait queue.
    queue.push(running);
    // Mark the running thread as blocked.
    running->setState(BLOCK);
    // Switch to another thread.
    switchThreads();
    enableInterrupts();
}

// Wake up one thread waiting on this condition variable.
void CondVar::signal()
{
    disableInterrupts();
    if (!queue.empty()) {
        TCB* next = queue.front();
        queue.pop();
        next->setState(READY);
        addToReady(next);
    }
    enableInterrupts();
}

// Wake up all threads waiting on this condition variable.
void CondVar::broadcast()
{
    disableInterrupts();
    while (!queue.empty()) {
        TCB* next = queue.front();
        queue.pop();
        next->setState(READY);
        addToReady(next);
    }
    enableInterrupts();
}
