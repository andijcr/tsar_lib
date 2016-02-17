//
// Created by andrea on 01/02/16.
//

#include <stddef.h>
#include <stdlib.h>
#include "tsar.h"
#include "lib/inmemory_logger.h"

struct tsar_t {
    int lol;
};


tsar_t *tsar_init() {
    tsar_t* t=malloc(sizeof(tsar_t));

    LOG("creato un tsar", 0);
    return t;
}

void tsar_destroy(tsar_t *t) {

    free(t);
    LOG("distrutto uno tsar", 0);
}

struct in_chan_t{
    int what;
};

in_chan_t *publisher_create(tsar_t *t, char *name, int name_l) {
    return NULL;
}

void publisher_remove(in_chan_t *t) {

}
