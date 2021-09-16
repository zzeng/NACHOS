#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include <semaphore.h>
#include "synch.h"
class RWLock{
private:
  int AR, WR, AW, WW;
  //pthread_cond_t okToRead, okToWrite;
  //pthread_mutex_t lock;
  Condition *okToRead, *okToWrite;
  Lock *lock;
public:
    RWLock();
    ~RWLock();
    //Reader
    void startRead();
    void doneRead();
    // Writer
    void startWrite();
    void  doneWrite();
};

#endif
