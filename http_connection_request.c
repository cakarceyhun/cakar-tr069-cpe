#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

#include "active.h"

#include "inform_queue.h"

void* http_connection_request_thread_main(void* arg_pv)
{
    fd_set fds;
    struct timeval tv;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    char buffer[256];

    struct arg_s *arg = (struct arg_s*) arg_pv;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
        perror("bind error\n");
        goto end;
    }

    if (listen(listenfd, 10)) {
        perror("listen error\n");
        goto end;
    }

    while(1)
    {
        pthread_mutex_lock(&arg->lock);
        if (arg->quit) {
            pthread_mutex_unlock(&arg->lock);
            break;
        }
        pthread_mutex_unlock(&arg->lock);

        FD_ZERO(&fds);
        FD_SET(listenfd, &fds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int retval = select(listenfd + 1, &fds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select error\n");
        } else if (FD_ISSET(listenfd, &fds)) {
            printf("connected\n");

            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

            read(connfd, buffer, 255);
            time_t now = time(NULL);
            struct tm *now_tm;
            static const char *week_days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            static const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Agu", "Sep", "Oct", "Nov", "Dec"};

            now_tm = gmtime(&now);
            snprintf(buffer, 128, "HTTP/1.1 200 OK\r\nDate: %s, %02d %s %d %02d:%02d:%02d GMT\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n",
                     week_days[now_tm->tm_wday], now_tm->tm_mday, months[now_tm->tm_mon], now_tm->tm_year + 1900, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
            write(connfd, buffer, strlen(buffer));

            inform_queue_send(&arg->queue, INFORM_TYPE_CONNECTION_REQUEST);

            close(connfd);
            sleep(1);
        } else {
            printf("timeout\n");
        }
     }

end:
    pthread_exit(NULL);
}
