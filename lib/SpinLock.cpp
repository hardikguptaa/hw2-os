#include "SpinLock.h"

SpinLock::SpinLock()
{
    // Nothing to do
}

// Acquire the lock. Spin until the lock is acquired if the lock is already held
void SpinLock::lock()
{
    // while (atomic_value.test_and_set(std::memory_order_acquire))
    while (atomic_value.test_and_set(&atomic_flag)) {
        // do nothing
    }
}
// Unlock the lock
void SpinLock::unlock()
{
    // TODO
    // atomic_value.clear(std::memory_order_release);
    atomic_value.clear(&atomic_flag);
}