//
// Created by andrea on 01/02/16.
//

#include <stddef.h>
#include <stdlib.h>
#include "tsar.h"
#include "lib/inmemory_logger.h"
#include "lib/list.h"
#include <pthread.h>
#include <memory.h>
#include <stdbool.h>

#define SMALL_STRING 15


struct tsar_t {

    pthread_rwlock_t l_channels;
    list_t* channels_map;
};


tsar_t *tsar_init() {
    tsar_t* t=malloc(sizeof(tsar_t));

    pthread_rwlock_init(&t->l_channels, NULL);
    t->channels_map = list_init();
    LOG("creato un tsar", (long) t);
    return t;
}

void tsar_destroy(tsar_t *t) {

    list_destroy(t->channels_map);
    pthread_rwlock_destroy(&t->l_channels);
    free(t);
    LOG("distrutto uno tsar", (long) t);
}

//small string optimization experiment
typedef struct {
    char* val;
    char opt[SMALL_STRING+1];
}t_string;

void copy_str(t_string* s, const char* source){
    int l=strlen(source);
    if(l>SMALL_STRING){
        s->val=malloc(sizeof(char)*l + 1);
    }else{
        s->val=s->opt;
    }
    strncpy(s->val, source, l);
}

void destroy_str(t_string* s){
    if(s->val != s->opt){
        free(s->val);
    }
}


struct in_chan_t{
    t_string name;
    list_t* subscribers;
};


in_chan_t* find_publisher(list_t* c_map, const char* name){
    iterator_t* i=iterator_init(c_map);
    in_chan_t* e=NULL;
    while((e=next(i)) != NULL){
        int order=strcmp(e->name.val, name);
        if(order==0){
            break;
        }
    }
    iterator_destroy(i);
    return e;
}

void put_publisher(list_t* c_map, const in_chan_t* c){
    addElement(c_map, c);
}

void remove_publisher(list_t* c_map, const in_chan_t* c){
    removeElement(c_map, c);
}


/*
 * if name already exists, return NULL
 * otherwise, in_chan_t* is a valid channel to publish
 * wrlock around find+put,
 *
 */
in_chan_t *publisher_create(tsar_t *t, const char *name) {
    in_chan_t* c =NULL;

    pthread_rwlock_wrlock(&t->l_channels);
    if(find_publisher(t->channels_map, name)==NULL) {
        in_chan_t *c = malloc(sizeof(in_chan_t));
        copy_str(&c->name, name);
        c->subscribers=list_init();
        put_publisher(t->channels_map, c);
    }
    pthread_rwlock_unlock(&t->l_channels);

    LOG("creato un canale", c);
    return c;
}

void publisher_remove(tsar_t *t, in_chan_t *c) {
    pthread_rwlock_wrlock(&t->l_channels);
    remove_publisher(t->channels_map, c);
    pthread_rwlock_unlock(&t->l_channels);

    list_destroy(c->subscribers);
    destroy_str(&c->name);
    free(c);
    LOG("distrutto un canale", t);
}
