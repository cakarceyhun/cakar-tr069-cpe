#ifndef XML_INFORM_GENERATOR_H_CEYHUNCAKAR_2021_12_20
#define XML_INFORM_GENERATOR_H_CEYHUNCAKAR_2021_12_20

#include "inform.h"

void *inform_queue_create();

int inform_queue_send(void **queue_vp, enum inform_type_e type);

void inform_queue_destroy(void **queue_vp);

void* inform_thread_main(void* queue);

#endif

