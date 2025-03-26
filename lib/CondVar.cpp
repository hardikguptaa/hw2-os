#include "CondVar.h"
#include "uthread_private.h"
#include <cassert>

CondVar::CondVar()
{
    // Nothing to do
}

// Release the lock and block this thread atomically. Thread is woken up when
// signalled or broadcasted
void CondVar::wait(Lock &lock)
{
    disableInterrupts();
    this->lock = &lock;
    queue.push(running);
    running->setState(BLOCK);
    lock._unlock();   
    switchThreads();
    enableInterrupts();
}

// Wake up a blocked thread if any is waiting, do nothing otherwise
void CondVar::signal()
{
    disableInterrupts();
    if (!queue.empty()) {
        TCB* next = queue.front();
        queue.pop();
        lock->_signal(next);
    }
    enableInterrupts();
}

// Wake up all blocked threads if any are waiting
void CondVar::broadcast()
{
    disableInterrupts();
    while (!queue.empty()) {
        TCB* next = queue.front();
        queue.pop();
        lock->_signal(next);
    }
    enableInterrupts();
}