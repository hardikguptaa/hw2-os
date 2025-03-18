/*
 *
 */

#include "TCB.h"
#include <cassert>

#define STACK_SIZE 32768
extern "C" void stub(void *(*start_routine)(void *), void *arg);

TCB::TCB(int tid, Priority pr, void *(*start_routine)(void *arg), void *arg, State state) : _tid(tid), _pr(pr), _quantum(0), _state(state), _lock_count(0)
{
    // TODO
    _stack = new char[STACK_SIZE];

    // 2. Prepare the context
    getcontext(&_context);

    _context.uc_stack.ss_sp    = _stack;
    _context.uc_stack.ss_size  = STACK_SIZE;
    _context.uc_stack.ss_flags = 0;
    _context.uc_link           = nullptr; // If thread returns from stub, it’s done

    // 3. Use makecontext to have this thread start in the stub, which in turn
    //    calls the real start_routine().
    makecontext(&_context,
                (void (*)())stub, // cast to the needed function signature
                2,
                start_routine,
                arg);
}

TCB::~TCB()
{
    // TODO
    // Free the stack
    delete[] _stack;
    _stack = nullptr;
}

void TCB::setState(State state)
{
    // TODO
    _state = state;
}

State TCB::getState() const
{
    // TODO
    return _state; // return statement added only to allow compilation (replace with correct code)
}

int TCB::getId() const
{
    // TODO
    return _tid; // return statement added only to allow compilation (replace with correct code)
}

Priority TCB::getPriority() const
{
    // TODO
    return _pr;  // return statement added only to allow compilation (replace with correct code)
}

void TCB::increaseQuantum()
{
    // TODO
    _quantum++;
}

int TCB::getQuantum() const
{
    // TODO
    return _quantum; // return statement added only to allow compilation (replace with correct code)
}

void TCB::increaseLockCount()
{
    // TODO
    _lock_count++;
}

void TCB::decreaseLockCount()
{
    // TODO
    _lock_count--;
}

int TCB::getLockCount()
{
    // TODO
    return _lock_count; // return statement added only to allow compilation (replace with correct code)
}

void TCB::increasePriority()
{
    // TODO
    // homework uses 3 levels: RED=highest, ORANGE=middle, GREEN=lowest,
    switch (_pr)
    {
        case RED:
            // Already highest, no change
            break;
        case ORANGE:
            _pr = RED;
            break;
        case GREEN:
            _pr = ORANGE;
            break;
    }
}

void TCB::decreasePriority()
{
    // TODO
    switch (_pr)
    {
        case RED:
            _pr = ORANGE;
            break;
        case ORANGE:
            _pr = GREEN;
            break;
        case GREEN:
            // Already lowest
            break;
    }
}
