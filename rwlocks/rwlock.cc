#include <stdio.h>
#include <pthread.h>
#include <iostream> //


#include "rwlock.h"

RWLock::RWLock() {
  AR = 0;
  WR = 0;
  AW = 0;
  WW = 0;
  lock = PTHREAD_MUTEX_INITIALIZER;
  okToRead = PTHREAD_COND_INITIALIZER;
  okToWrite = PTHREAD_COND_INITIALIZER;
}
RWLock::~RWLock() { }
void RWLock::startRead() {
  pthread_mutex_lock(&lock);
  while ((AW + WW) > 0) {
    WR++;
    pthread_cond_wait(&okToRead, &lock);
    WR--;
  }
  AR++;
  pthread_mutex_unlock(&lock);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&lock);
    AR--;
    if (AR == 0 && WW > 0){
        pthread_cond_signal(&okToWrite);
    }
    pthread_mutex_unlock(&lock);
}

void RWLock::startWrite() {
  pthread_mutex_lock(&lock);
  while ((AW + AR) > 0) {
    WW++;
    pthread_cond_wait(&okToWrite, &lock);
    WW--;
  }
  AW++;
  pthread_mutex_unlock(&lock);

}
void RWLock::doneWrite() {
    pthread_mutex_lock(&lock);
    AW--;
    if (WW > 0){
        pthread_cond_signal(&okToWrite);
    }
    else if (WR > 0){
        pthread_cond_broadcast(&okToRead);
    }
    pthread_mutex_unlock(&lock);
 }
