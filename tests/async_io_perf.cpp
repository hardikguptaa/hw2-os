/*
 * Purpose:
 *   This program measures and compares the performance of synchronous I/O versus 
 *   asynchronous I/O using the provided async_read() and async_write() functions.
 *
 *   It uses a large buffer and measures the time taken to perform write and read
 *   operations both synchronously (using write() and read()) and asynchronously 
 *   (using async_write() and async_read()), while yielding control between I/O 
 *   operations using the user-level thread library (uthread).
 *
 *   Note: BUFFER_SIZE is defined as (4096 * 4096) which equals 16 MB. If you intended
 *   1 MB, change the definition to (1024 * 1024).
 */

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
 #define BUFFER_SIZE (4096 * 4096)  // 16 MB (change to 1024*1024 for 1 MB)
 
 
 // Global buffers for I/O.
 char writeBuffer[BUFFER_SIZE];
 char readBuffer[BUFFER_SIZE];
 
 /*
  * Function: fillBuffer
  * Purpose: Initializes writeBuffer with a repeated alphabetical pattern.
  * Input: None.
  * Output: Global writeBuffer is filled with characters 'A' to 'Z', repeated.
  */
 void fillBuffer() {
     for (int i = 0; i < BUFFER_SIZE - 1; i++) {
         writeBuffer[i] = 'A' + (i % 26);
     }
     writeBuffer[BUFFER_SIZE - 1] = '\0';
 }
 
 /*
  * Function: main
  * Purpose:
  *   - Initializes the uthread library.
  *   - Fills the write buffer with data.
  *   - Performs synchronous I/O tests (using write() and read()) on a test file,
  *     measuring the time taken.
  *   - Performs asynchronous I/O tests (using async_write() and async_read()) on the
  *     same file, measuring the time taken.
  *   - Prints out the number of bytes transferred and the elapsed time for each operation.
  *
  * Input: No command-line parameters required for this test.
  * Output: Printed performance results for synchronous and asynchronous I/O.
  */
 int main() {
     // Initialize the user-level thread library.
     if (uthread_init(UTHREAD_TIME_QUANTUM) != 0) {
         cerr << "Error: uthread_init failed!" << endl;
         return 1;
     }
     
     const char* filename = "io_test_file.txt";
     fillBuffer();
     
     // --- Synchronous I/O Test ---
     // Open file for writing synchronously.
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
     // Open file for asynchronous writing.
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
     
     // Open file for asynchronous reading.
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
     cout << "Synchronous Write: " << bytes_sync_written << " bytes in " 
          << sync_write_time << " seconds." << endl;
     cout << "Synchronous Read : " << bytes_sync_read  << " bytes in " 
          << sync_read_time  << " seconds." << endl;
     cout << "Asynchronous Write: " << bytes_async_written << " bytes in " 
          << async_write_time << " seconds." << endl;
     cout << "Asynchronous Read : " << bytes_async_read  << " bytes in " 
          << async_read_time  << " seconds." << endl;
     
     return 0;
 }
 