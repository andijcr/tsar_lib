//
// Created by andrea on 05/02/16.
//

#include "inmemory_logger.h"

inmem_event inmem_buffer[INMEM_BUFFER_SIZE];
atomic_ulong inmem_pos=0;

extern void inmem_log(const char* msg, long param){
    // Get next event index
    unsigned long index=atomic_fetch_add(&inmem_pos, 1);
    // Write an event at this index
    inmem_event* e = inmem_buffer + (index & (INMEM_BUFFER_SIZE - 1));  // Wrap to buffer size
    e->tid = pthread_self();           // Get thread ID
    e->msg = msg;
    e->param = param;
    e->seq = index;
}
