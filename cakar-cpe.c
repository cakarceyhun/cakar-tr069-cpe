#include "active.h"
#include "datamodel.h"
#include "inform_queue.h"
#include <stdio.h>
#include <stdlib.h>

#include "http_connection_request.h"

#include "xml_generator.h"

int main(int argc, char** argv)
{
    pthread_t inform_thread;
    pthread_t connection_request_thread;
    struct arg_s arg;
    void *thread_ret;
    int ret = 0;

    pthread_mutex_init(&arg.lock, NULL);
    arg.quit = 0;

    create_database();

    arg.queue = inform_queue_create();

    if (pthread_create(&inform_thread, NULL, inform_thread_main, &arg) != 0) {
        perror("pthread_create error");
        goto fail;
    }

    if (pthread_create(&connection_request_thread, NULL, http_connection_request_thread_main, &arg) != 0) {
        perror("pthread_create error");
        goto fail;
    }

    inform_queue_send(&arg.queue, INFORM_TYPE_BOOTSTRAP);

    char buffer[256] = "";
    printf("> ");
    scanf ("%64s", buffer);
    printf("buffer=%s\n", buffer);

    pthread_mutex_lock(&arg.lock);
    arg.quit = 1;
    inform_queue_send(&arg.queue, INFORM_TYPE_NONE);
    pthread_mutex_unlock(&arg.lock);

    if (pthread_join(inform_thread, &thread_ret) != 0) {
        perror("pthread_create error");
        goto fail;
    }

    if (pthread_join(connection_request_thread, &thread_ret) != 0) {
        perror("pthread_create() error");
        goto fail;
    }

    inform_queue_destroy(&arg.queue);

    goto success;

fail:
    ret = 1;

success:
    
    pthread_mutex_destroy(&arg.lock);
    pthread_exit(0);
    return ret;
}
