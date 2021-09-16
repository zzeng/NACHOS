#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include "rwlock.h"
#include "synch.h"

RWLock::RWLock() {
  AR = 0;
  WR = 0;
  AW = 0;
  WW = 0;
  //lock = PTHREAD_MUTEX_INITIALIZER;
  //okToRead = PTHREAD_COND_INITIALIZER;
  //okToWrite = PTHREAD_COND_INITIALIZER;
  lock = new Lock("lock");
  okToRead = new Condition("okToRead");
  okToWrite = new Condition("okToWrite");
}
RWLock::~RWLock() { }
void RWLock::startRead() {
  //pthread_mutex_lock(&lock);
  lock->Acquire();
  while ((AW + WW) > 0) {
    WR++;
    //pthread_cond_wait(&okToRead, &lock);
    okToRead->Wait(lock);
    WR--;
  }
  AR++;
  //pthread_mutex_unlock(&lock);
  lock->Release();
}

void RWLock::doneRead() {
  //pthread_mutex_lock(&lock);
  lock->Acquire();
  AR--;
  if (AR == 0 && WW > 0){
    //pthread_cond_signal(&okToWrite);
    okToWrite->Signal(lock);
  }
  //pthread_mutex_unlock(&lock);
  lock->Release();
}

void RWLock::startWrite() {
  //pthread_mutex_lock(&lock);
  lock->Acquire();
  while ((AW + AR) > 0) {
    WW++;
    //pthread_cond_wait(&okToWrite, &lock);
    okToWrite->Wait(lock);
    WW--;
  }
  AW++;
  //pthread_mutex_unlock(&lock);
  lock->Release();
}
void RWLock::doneWrite() {
  //pthread_mutex_lock(&lock);
  lock->Acquire();
  AW--;
  if (WW > 0){
    //pthread_cond_signal(&okToWrite);
    okToWrite->Signal(lock);
  }
  else if (WR > 0){
    //pthread_cond_broadcast(&okToRead);
    okToRead->Broadcast(lock);
  }
  //pthread_mutex_unlock(&lock);
  lock->Release();
 }
