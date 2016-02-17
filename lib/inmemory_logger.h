//
// Created by andrea on 04/02/16.
// inspired by preshing.com/20120522/lightweight-in-memory-logging/
//

#ifndef TSAR_THREADSAFEROUTER_INMEMORYLOGGER_H
#define TSAR_THREADSAFEROUTER_INMEMORYLOGGER_H

#include <stdatomic.h>
#include <pthread.h>

typedef struct{
        pthread_t tid;           // Thread ID
        const char* msg;    // Message string
        long param;         // A parameter which can mean anything you want
        unsigned long seq;  // sequence number, usefull when there is a wrap around in the buffer
    } inmem_event;

#define INMEM_BUFFER_SIZE 65536   // Must be a power of 2
extern inmem_event inmem_buffer[INMEM_BUFFER_SIZE];
extern atomic_ulong inmem_pos;

void inmem_log(const char* msg, long param);


#ifdef INMEM_LOG
    #define LOG(m, p) inmem_log(m, p)
#else
    #define LOG(m, p)
#endif

#endif //TSAR_THREADSAFEROUTER_INMEMORYLOGGER_H
