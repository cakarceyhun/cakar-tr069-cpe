#ifndef ACTIVE_H_CEYHUN_CAKAR_2021_12_24
#define ACTIVE_H_CEYHUN_CAKAR_2021_12_24

#include <pthread.h>

struct arg_s
{
    pthread_mutex_t lock;
    int quit;
    void *queue;
};

#endif

