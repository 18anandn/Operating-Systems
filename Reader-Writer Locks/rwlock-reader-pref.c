#include "rwlock.h"



void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
  rw->readers = 0;
  rw->writers = 0;
  rw->writing = false;
  rw->reading = 0;
  pthread_cond_init(&rw->reader_wait, NULL);
  pthread_cond_init(&rw->writer_wait, NULL);
  pthread_mutex_init(&rw->lock, NULL);
}

void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  rw->readers++;
  while(rw->writing) pthread_cond_wait(&rw->reader_wait, &rw->lock);
  pthread_mutex_unlock(&rw->lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  rw->readers--;
  pthread_cond_broadcast(&rw->writer_wait);
  pthread_mutex_unlock(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  while(rw->readers > 0 || rw->writing) pthread_cond_wait(&rw->writer_wait, &rw->lock);
  rw->writing = true;
  pthread_mutex_unlock(&rw->lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  rw->writing = false;
  pthread_cond_broadcast(&rw->reader_wait);
  pthread_cond_broadcast(&rw->writer_wait);
  pthread_mutex_unlock(&rw->lock);
}
