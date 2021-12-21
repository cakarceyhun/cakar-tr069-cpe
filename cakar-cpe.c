#include "datamodel.h"
#include "inform_queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "http_connection_request.h"


int main(int argc, char** argv)
{
    pthread_t inform_thread;
    pthread_t connection_request_thread;
    void *ret;
  
    create_database();

    void *queue = inform_queue_create();

    if (pthread_create(&inform_thread, NULL, inform_thread_main, queue) != 0) {
	perror("pthread_create error");
	exit(1);
    }

    if (pthread_create(&connection_request_thread, NULL, http_connection_request_thread_main, queue) != 0) {
	perror("pthread_create error");
	exit(1);
    }

    inform_queue_send(&queue, INFORM_TYPE_BOOTSTRAP);

    if (pthread_join(inform_thread, &ret) != 0) {
	perror("pthread_join error");
	exit(3);
    }

    if (pthread_join(connection_request_thread, &ret) != 0) {
	perror("pthread_join error");
	exit(3);
    }

    inform_queue_destroy(&queue);
    
    return 0;
}
