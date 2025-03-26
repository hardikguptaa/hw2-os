#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "../lib/uthread.h"      // Headers in lib/
#include "../lib/async_io.h"

using namespace std;

const char* TEST_FILENAME = "async_test.txt";
const char* MESSAGE = "Hello from async I/O!\n";

void* writerFunc(void* arg) {
    int fd = open(TEST_FILENAME, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        cerr << "Error opening file for writing." << endl;
        return nullptr;
    }
    ssize_t bytes_written = async_write(fd, (void*)MESSAGE, strlen(MESSAGE), 0);
    close(fd);
    cout << "[Async I/O Test] Writer wrote " << bytes_written << " bytes." << endl;
    cout.flush();
    return nullptr;
}

void* readerFunc(void* arg) {
    int fd = open(TEST_FILENAME, O_RDONLY);
    if (fd < 0) {
        cerr << "Error opening file for reading." << endl;
        return nullptr;
    }
    char buf[256];
    memset(buf, 0, sizeof(buf));
    ssize_t bytes_read = async_read(fd, buf, sizeof(buf) - 1, 0);
    close(fd);
    cout << "[Async I/O Test] Reader read " << bytes_read << " bytes: " << buf << endl;
    cout.flush();
    return nullptr;
}

int main() {
    if (uthread_init(10000) != 0) {
        cerr << "Error: uthread_init failed!" << endl;
        return 1;
    }
    int writer_tid = uthread_create(writerFunc, nullptr);
    int reader_tid = uthread_create(readerFunc, nullptr);
    cout << "Created writer thread with id " << writer_tid << endl;
    cout << "Created reader thread with id " << reader_tid << endl;
    uthread_join(writer_tid, nullptr);
    uthread_join(reader_tid, nullptr);
    return 0;
}
