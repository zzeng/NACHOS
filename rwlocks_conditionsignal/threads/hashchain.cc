/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 * Use fine-grain rwlock or lock
 **/



#include <iostream>
#include <unistd.h>
#include "rwlock.h"
#include "hashchain.h"
#include "synch.h"

//NOYIELD disables the yields used to help uncover synchronization bugs
//  - the yields are useful for uncovering bugs, but performance testing may be easier without them
//#define NOYIELD 1

#ifndef NOYIELD
#ifdef NACHOS
#include "system.h" //for currentThread->Yield()
#define YIELD() currentThread->Yield()
#else //else pthreads
#define YIELD() pthread_yield()
#endif
#else //else NOYIELD
#define YIELD() do{}while(0)
#endif

//You don't need to use these, but if you use these macros then you only need to insert one line of code at each synchronization point down in the main code
//You can define the behavior for each lock call like follows:
//  #define START_READ() lock.lock()
//  #define END_READ() lock.unlock()
#ifdef NOLOCK
//no synchronization code
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#elif defined P1_SEMAPHORE //using nachos semaphore
#define START_READ() sema->P();
#define END_READ() sema->V();
#define START_WRITE() sema->P();
#define END_WRITE() sema->V();
#elif defined P1_LOCK //using our implemented nachos lock
#define START_READ() lock->Acquire();
#define END_READ() lock->Release();
#define START_WRITE() lock->Acquire();
#define END_WRITE() lock->Release();
#elif defined P1_RWLOCK //using our rwlock
#define START_READ() rwlocks[hash].startRead();
#define END_READ() rwlocks[hash].doneRead();
#define START_WRITE() rwlocks[hash].startWrite();
#define END_WRITE() rwlocks[hash].doneWrite();
#else //else behave like NOLOCK (no option passed)
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#endif

LinkedHashEntry:: LinkedHashEntry(int key1, int value1) {
  this->key = key1;
  this->value = value1;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return key;
}
int 
LinkedHashEntry:: getValue() {
  return value;
}

void 
LinkedHashEntry:: setValue(int value1) {
  this->value = value1;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *next1) {
  this->next = next1;
}

const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;
#ifdef P1_SEMAPHORE
  sema = new Semaphore("semaphore", 1);
#elif defined P1_LOCK
  lock = new Lock("lock");
#elif defined P1_RWLOCK
  rwlocks = new RWLock[TABLE_SIZE];
  for(int i = 0; i < TABLE_SIZE; i++) {
    RWLock rwlocks[i];
  }
#endif
}

int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    return -1;
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry != NULL && entry->getKey() != key) {
      entry = entry->getNext();
      YIELD();
    }
    if (entry == NULL) {
      YIELD();
      return -1;
    } else { 
      YIELD();
      return entry->getValue();
    }
  }
  return -1; //should never get here (just for complaining compilers)
}

void
HashMap::_put(int key, int value) { //internal put() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    table[hash] = new LinkedHashEntry(key, value);
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      entry->setValue(value);
    } else {
      YIELD();
      entry->setNext(new LinkedHashEntry(key, value));
    }
  }
  YIELD();
}

int 
HashMap::get(int key1) { //TODO: make this function threadsafe
  int hash = (key1 % TABLE_SIZE); //may be needed for START_/END_ macros
  //usleep(10);
  START_READ();
  int ret =  _get(key1);
  END_READ();
  return ret;
}

void 
HashMap::put(int key1, int value1) { //TODO: make this function threadsafe
  int hash = (key1 % TABLE_SIZE); //may be needed for START_/END_ macros
  START_WRITE();
  _put(key1,value1);
  END_WRITE();
}


void
HashMap::remove(int key) { //TODO: make this function threadsafe
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  if (table[hash] != NULL) {
    YIELD();
    LinkedHashEntry *prevEntry = NULL;
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      prevEntry = entry;
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      if (prevEntry == NULL) {
        LinkedHashEntry *nextEntry = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        table[hash] = nextEntry;
      } else {
        LinkedHashEntry *next = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        prevEntry->setNext(next);
      }
    }
  }
  END_WRITE();
}

HashMap:: ~HashMap() {
  for (int hash = 0; hash < TABLE_SIZE; hash++){
    if (table[hash] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[hash];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
  }
  delete[] table;
#ifdef P1_SEMAPHORE
  delete sema;
#elif defined P1_LOCK
  delete lock;
#elif defined P1_RWLOCK
  delete [] rwlocks;
#endif
}


void
HashMap::increment(int key, int value) { //TODO: make this function threadsafe
  int hash = (key % TABLE_SIZE); //may be needed for START_/END_ macros
  START_WRITE();
  _put(key,_get(key)+value);
  END_WRITE();
}
