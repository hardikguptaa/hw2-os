#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "../lib/uthread.h"      // uthread functions; required for async I/O test
#include "../lib/async_io.h"     // Your async I/O implementations

using namespace std;
using namespace std::chrono;

#define UTHREAD_TIME_QUANTUM 10000
#define BUFFER_SIZE (1024 * 1024)  // 1 MB

// Create a large buffer to use for I/O.
char writeBuffer[BUFFER_SIZE];
char readBuffer[BUFFER_SIZE];

void fillBuffer() {
    // Fill writeBuffer with a repeated pattern.
    for (int i = 0; i < BUFFER_SIZE - 1; i++) {
        writeBuffer[i] = 'A' + (i % 26);
    }
    writeBuffer[BUFFER_SIZE - 1] = '\0';
}

int main() {
    // Initialize uthread library.
    if (uthread_init(UTHREAD_TIME_QUANTUM) != 0) {
        cerr << "Error: uthread_init failed!" << endl;
        return 1;
    }
    
    const char* filename = "io_test_file.txt";
    fillBuffer();
    
    // --- Synchronous I/O Test ---
    // Open file for writing (synchronously).
    int fd_sync = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_sync < 0) {
        cerr << "Error opening file for synchronous writing." << endl;
        return 1;
    }
    auto start_sync_write = high_resolution_clock::now();
    ssize_t bytes_sync_written = write(fd_sync, writeBuffer, strlen(writeBuffer));
    auto end_sync_write = high_resolution_clock::now();
    close(fd_sync);
    double sync_write_time = duration_cast<duration<double>>(end_sync_write - start_sync_write).count();
    
    // Open file for reading synchronously.
    fd_sync = open(filename, O_RDONLY);
    if (fd_sync < 0) {
        cerr << "Error opening file for synchronous reading." << endl;
        return 1;
    }
    auto start_sync_read = high_resolution_clock::now();
    ssize_t bytes_sync_read = read(fd_sync, readBuffer, BUFFER_SIZE - 1);
    auto end_sync_read = high_resolution_clock::now();
    close(fd_sync);
    double sync_read_time = duration_cast<duration<double>>(end_sync_read - start_sync_read).count();
    
    // --- Asynchronous I/O Test ---
    // For async I/O, we use our async_write and async_read functions.
    // First, open file for writing.
    int fd_async = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_async < 0) {
        cerr << "Error opening file for asynchronous writing." << endl;
        return 1;
    }
    auto start_async_write = high_resolution_clock::now();
    ssize_t bytes_async_written = async_write(fd_async, writeBuffer, strlen(writeBuffer), 0);
    auto end_async_write = high_resolution_clock::now();
    close(fd_async);
    double async_write_time = duration_cast<duration<double>>(end_async_write - start_async_write).count();
    
    // Open file for reading asynchronously.
    fd_async = open(filename, O_RDONLY);
    if (fd_async < 0) {
        cerr << "Error opening file for asynchronous reading." << endl;
        return 1;
    }
    auto start_async_read = high_resolution_clock::now();
    ssize_t bytes_async_read = async_read(fd_async, readBuffer, BUFFER_SIZE - 1, 0);
    auto end_async_read = high_resolution_clock::now();
    close(fd_async);
    double async_read_time = duration_cast<duration<double>>(end_async_read - start_async_read).count();
    
    // Print results.
    cout << "Synchronous Write: " << bytes_sync_written << " bytes in " << sync_write_time << " seconds." << endl;
    cout << "Synchronous Read : " << bytes_sync_read  << " bytes in " << sync_read_time  << " seconds." << endl;
    cout << "Asynchronous Write: " << bytes_async_written << " bytes in " << async_write_time << " seconds." << endl;
    cout << "Asynchronous Read : " << bytes_async_read  << " bytes in " << async_read_time  << " seconds." << endl;
    
    return 0;
}
