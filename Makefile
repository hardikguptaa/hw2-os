CC = g++
CXXFLAGS = -g --std=c++14 -I./lib
LDFLAGS = -lrt

DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h async_io.h

# Use instructor’s solution objects for uthread and TCB when desired.
OBJ_SOLN = ./solution/TCB_soln.o ./solution/uthread_soln.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o
# Otherwise, use our own (if you choose not to link with solution objects):
OBJ = ./lib/TCB.o ./lib/uthread.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o

# Test files
MAIN_OBJ_SYNC_DEMO = ./tests/uthread_sync_demo.o
PERF_LOCK_OBJ = ./tests/lock_vs_spin_perf.o
PERF_ASYNC_OBJ = ./tests/async_io_perf.o
SPINLOCK_TEST_OBJ = ./tests/spinlock_test.o

# Pattern rules:
./lib/%.o: ./lib/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

./tests/%.o: ./tests/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

# Default target uses our own uthread (if you want to test your code)
all: uthread-sync-demo-from-soln perf_lock perf_async spinlock_test

# Build the producer-consumer demo using the solution objects for uthread and TCB.
uthread-sync-demo-from-soln: $(OBJ_SOLN) $(MAIN_OBJ_SYNC_DEMO)
	$(CC) -o uthread-sync-demo $^ $(LDFLAGS)

# Performance evaluation: Lock vs SpinLock test.
perf_lock: $(OBJ_SOLN) $(PERF_LOCK_OBJ)
	$(CC) -o perf_lock $^ $(LDFLAGS)

# Performance evaluation: Synchronous vs Asynchronous I/O.
perf_async: $(OBJ_SOLN) $(PERF_ASYNC_OBJ)
	$(CC) -o perf_async $^ $(LDFLAGS)

spinlock_test: $(OBJ_SOLN) $(SPINLOCK_TEST_OBJ)
	$(CC) -o spinlock_test $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f ./lib/*.o
	rm -f ./tests/*.o
	rm -f uthread-sync-demo perf_lock perf_async spinlock_test
	rm -f perf