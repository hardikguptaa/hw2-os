#include "Lock.h"
#include "uthread_private.h"
#include <cassert>

Lock::Lock() : held(false)
{
    // Nothing to do
}

// Attempt to acquire lock. Grab lock if available, otherwise thread is
// blocked until the lock becomes available
void Lock::lock()
{ 
    disableInterrupts();
    if(held) {
        // TCB* current = running; //need to find the current thread
        // entrance_queue.push(current);  //might need to set the state of the threads
        // switchThreads();    
        
        // If lock is already held, block the current (running) thread
        running->setState(BLOCK);
        entrance_queue.push(running);
        switchThreads();
        // When we return here, this thread has been woken,
        // so now we can safely set 'held = true'.
        // held = true;
    }
    else {
        held = true;
    }
    enableInterrupts();
}

// Unlock the lock. Wake up a blocked thread if any is waiting to acquire the
// lock and hand off the lock
void Lock::unlock()
{
    // TODO
    disableInterrupts();
    _unlock(); // Confirm this once
    enableInterrupts();
}

// Unlock the lock while interrupts have already been disabled
// NOTE: This function should NOT be used by user code. It is only to be used
//       by uthread library code
void Lock::_unlock()
{   
    if(!signaled_queue.empty()) {
        TCB* next = signaled_queue.front();
        signaled_queue.pop();
        next->setState(READY);
        addToReady(next);
    }
    else if(!entrance_queue.empty()) {
        TCB* next = entrance_queue.front();
        entrance_queue.pop();
        next->setState(READY);
        addToReady(next);
    }
    else{
        //No one is waiting, so free the lock
        held = false;
    }
    
}

// Let the lock know that it should switch to this thread after the lock has
// been released (following Mesa semantics)
void Lock::_signal(TCB *tcb)
{
    // TODO
    signaled_queue.push(tcb);
}
