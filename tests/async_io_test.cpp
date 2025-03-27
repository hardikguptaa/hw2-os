/*
 * Purpose:
 *   This program demonstrates the asynchronous I/O functionality of the user-level thread library.
 *   It creates two threads: one that writes a fixed message to a file asynchronously and one that reads
 *   the message back asynchronously. The writer uses async_write() to perform a non-blocking file write,
 *   while the reader uses async_read() to perform a non-blocking file read, yielding control between I/O
 *   operations with uthread_yield(). This test verifies that asynchronous I/O operations allow concurrent
 *   thread execution without blocking the entire process.
 */
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "../lib/uthread.h"
#include "../lib/async_io.h"

using namespace std;

const char* TEST_FILE = "async_io_test.txt";
const char* MESSAGE = "Hello from async I/O!\n";

void* writerFunc(void* arg) {
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        cerr << "[Writer] Error opening file." << endl;
        return nullptr;
    }
    ssize_t bytes = async_write(fd, (void*)MESSAGE, strlen(MESSAGE), 0);
    close(fd);
    cout << "[Writer] Wrote " << bytes << " bytes to " << TEST_FILE << endl;
    return nullptr;
}

void* readerFunc(void* arg) {
    // Possibly yield to let the writer run first
    for (int i = 0; i < 5; i++) uthread_yield(); 

    int fd = open(TEST_FILE, O_RDONLY);
    if (fd < 0) {
        cerr << "[Reader] Error opening file." << endl;
        return nullptr;
    }
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes = async_read(fd, buffer, sizeof(buffer) - 1, 0);
    close(fd);
    cout << "[Reader] Read " << bytes << " bytes: " << buffer << endl;
    return nullptr;
}

int main() {
    if (uthread_init(10000) < 0) {
        cerr << "Error: uthread_init failed." << endl;
        return 1;
    }
    int wTid = uthread_create(writerFunc, nullptr);
    int rTid = uthread_create(readerFunc, nullptr);

    cout << "[Main] Created writerTid=" << wTid << ", readerTid=" << rTid << endl;

    uthread_join(wTid, nullptr);
    uthread_join(rTid, nullptr);

    return 0;
}
