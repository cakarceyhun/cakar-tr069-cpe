#include "active.h"
#include "inform_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

struct inform_queue_item_s
{
    enum inform_type_e type;
    struct inform_queue_item_s *next;
};

struct inform_queue_s
{
    struct inform_queue_item_s *start;
    struct inform_queue_item_s *end;
    size_t length;
    sem_t sem;
};

void *inform_queue_create()
{
    struct inform_queue_s *queue = calloc(1, sizeof(struct inform_queue_s));
    if (!queue) {
	return NULL;
    }
    
    if (sem_init(&queue->sem, 0, 0) != 0) {
	return NULL;
    }

    return (struct inform_queue_s *) queue;
}

int inform_queue_send(void **queue_vp, enum inform_type_e type)
{
    struct inform_queue_s *queue = *((struct inform_queue_s **) queue_vp);
    
    struct inform_queue_item_s* item = calloc(1, sizeof(struct inform_queue_item_s));
    if (!item) {
	return -1;
    }
    item->type = type;

    if (!queue->start) {
	queue->start = item;
	queue->end = item;
    } else {
	queue->end->next = item;
	queue->end = item;
    }

    ++queue->length;
    sem_post(&queue->sem);

    return 0;
}

void inform_queue_destroy(void **queue_vp)
{
    struct inform_queue_s *queue = *((struct inform_queue_s **) queue_vp);

    if (queue) {
	sem_destroy(&queue->sem);
	
	struct inform_queue_item_s* item = queue->start;
	while (item) {
	    struct inform_queue_item_s* next = item->next;
	    free(item);
	    item = next;
	}

	free(queue);
	*((struct inform_queue_s **) queue_vp) = NULL;
	
    }
}

enum inform_type_e inform_queue_receive(void **queue_vp)
{
    enum inform_type_e inform_type;
    
    struct inform_queue_s *queue = *((struct inform_queue_s **) queue_vp);

    sem_wait(&queue->sem);
    --queue->length;

    inform_type = queue->start->type;
    struct inform_queue_item_s* item = queue->start->next;
    free(queue->start);
    queue->start = item;

    return inform_type;
}


void* inform_thread_main(void* arg_pv)
{
    struct arg_s *arg = (struct arg_s*) arg_pv;

    while (1) {
        enum inform_type_e type = inform_queue_receive(&arg->queue);

        pthread_mutex_lock(&arg->lock);
        if (arg->quit) {
            pthread_mutex_unlock(&arg->lock);
            break;
        }
        pthread_mutex_unlock(&arg->lock);

        inform(type);
    }
    
    pthread_exit(NULL);
}
