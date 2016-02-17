//
// Created by andrea on 01/02/16.
//

#ifndef TSAR_THREADSAFEROUTER_TSAR_H
#define TSAR_THREADSAFEROUTER_TSAR_H

//forward declaration and typedef in one go!
typedef struct tsar_t tsar_t;

tsar_t* tsar_init();
void tsar_destroy(tsar_t*);

typedef struct in_chan_t in_chan_t;

in_chan_t* publisher_create(tsar_t* t, char* name, int name_l );
void publisher_remove(in_chan_t*);

#endif //TSAR_THREADSAFEROUTER_TSAR_H
