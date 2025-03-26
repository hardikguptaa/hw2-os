#include "async_io.h"
#include "uthread.h"
#include <aio.h>
#include <errno.h>
#include <string.h>

// Carry out an asynchronous read request where this thread will be blocked
// while servicing the read but other ready threads will be scheduled
// Input:
// - fd: File descriptor
// - buf: Buffer to store read in
// - count: Number of bytes to read
// - offset: File offset to start at
// Output:
// - Number of bytes read on success, -1 on failure
ssize_t async_read(int fd, void *buf, size_t count, int offset)
{
    struct aiocb async_read_req;
    memset(&async_read_req, 0, sizeof(struct aiocb)); // Zero out the struct to avoid potential issues

    async_read_req.aio_fildes = fd;    // File descriptor
    async_read_req.aio_buf = buf;      // Buffer to store read data
    async_read_req.aio_nbytes = count; // Number of bytes to read
    async_read_req.aio_offset = offset; // Offset in file

    if (aio_read(&async_read_req) == -1) {
        return -1; // Return -1 if the request couldn't be initiated
    }

    // Wait for the read operation to complete while allowing other threads to execute
    while (aio_error(&async_read_req) == EINPROGRESS) {
        uthread_yield();
    }

    // If the read operation completed successfully, return the number of bytes written.
    // Otherwise, return -1.
    return (aio_error(&async_read_req) == 0) ? aio_return(&async_read_req) : -1;
}

// Carry out an asynchronous write request where this thread will be blocked
// while servicing the write but other ready threads will be scheduled
// Input:
// - fd: File descriptor
// - buf: Buffer containing data to write to file
// - count: Number of bytes to write
// - offset: File offset to start at
// Output:
// - Number of bytes written on success, -1 on failure
ssize_t async_write(int fd, void *buf, size_t count, int offset)
{
    struct aiocb async_write_req;
    memset(&async_write_req, 0, sizeof(struct aiocb));

    async_write_req.aio_fildes = fd;
    async_write_req.aio_buf = buf;      
    async_write_req.aio_nbytes = count;
    async_write_req.aio_offset = offset;

    if (aio_write(&async_write_req) == -1) {
        return -1;
    }

    // Wait for the write operation to complete while allowing other threads to execute
    while (aio_error(&async_write_req) == EINPROGRESS) {
        uthread_yield();
    }

    // If the write operation completed successfully, return the number of bytes written.
    // Otherwise, return -1.
    return (aio_error(&async_write_req) == 0) ? aio_return(&async_write_req) : -1;
}
