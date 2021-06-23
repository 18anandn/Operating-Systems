#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

struct read_write_lock{
    int readers;
    int writers;
    int reading;
    bool writing;
    pthread_mutex_t lock;
    pthread_cond_t reader_wait;
    pthread_cond_t writer_wait;
};



void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
