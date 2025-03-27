CC = g++
CXXFLAGS = -g --std=c++14 -I./lib
LDFLAGS = -lrt

DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h async_io.h

# Use instructor’s solution objects for uthread and TCB when desired.
OBJ_SOLN = ./solution/TCB_soln.o ./solution/uthread_soln.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o

# Otherwise, use our own (if you want/need):
OBJ = ./lib/TCB.o ./lib/uthread.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o

# Test files / object files
MAIN_OBJ_SYNC_DEMO = ./tests/uthread_sync_demo.o
PERF_LOCK_OBJ      = ./tests/lock_vs_spin_perf.o
PERF_ASYNC_OBJ     = ./tests/async_io_perf.o
SPINLOCK_TEST_OBJ  = ./tests/spinlock_test.o

# NEW: Add these for lock_condvar_test.cpp and async_io_test.cpp
LOCK_CONDVAR_TEST_OBJ = ./tests/lock_condvar_test.o
ASYNC_IO_TEST_OBJ     = ./tests/async_io_test.o

# Pattern rules:
./lib/%.o: ./lib/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

./tests/%.o: ./tests/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

# Default target: build everything using the solution objects
all: uthread-sync-demo-from-soln perf_lock perf_async spinlock_test lock_condvar_test async_io_test

# Build the producer-consumer demo (solution objects for uthread/TCB).
uthread-sync-demo-from-soln: $(OBJ_SOLN) $(MAIN_OBJ_SYNC_DEMO)
	$(CC) -o uthread-sync-demo $^ $(LDFLAGS)

# Performance evaluation: Lock vs SpinLock test.
perf_lock: $(OBJ_SOLN) $(PERF_LOCK_OBJ)
	$(CC) -o perf_lock $^ $(LDFLAGS)

# Performance evaluation: Synchronous vs Asynchronous I/O.
perf_async: $(OBJ_SOLN) $(PERF_ASYNC_OBJ)
	$(CC) -o perf_async $^ $(LDFLAGS)

# SpinLock test
spinlock_test: $(OBJ_SOLN) $(SPINLOCK_TEST_OBJ)
	$(CC) -o spinlock_test $^ $(LDFLAGS)

# NEW: Lock + CondVar test
lock_condvar_test: $(OBJ_SOLN) $(LOCK_CONDVAR_TEST_OBJ)
	$(CC) -o lock_condvar_test $^ $(LDFLAGS)

# NEW: Async I/O test
async_io_test: $(OBJ_SOLN) $(ASYNC_IO_TEST_OBJ)
	$(CC) -o async_io_test $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f ./lib/*.o
	rm -f ./tests/*.o
	rm -f uthread-sync-demo perf_lock perf_async spinlock_test lock_condvar_test async_io_test
	rm -f perf
