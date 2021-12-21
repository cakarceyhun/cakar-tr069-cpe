#include "datamodel.h"
#include "inform_queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>


void* http_connection_request_thread_main(void* queue)
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char buffer[256];
    int ret = 0;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080); 

    ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    ret = listen(listenfd, 10); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

	ret = read(connfd, buffer, 255);
	time_t now = time(NULL);
	struct tm *now_tm;
	static const char *week_days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; 
	static const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Agu", "Sep", "Oct", "Nov", "Dec"};
					 
	now_tm = gmtime(&now);
	snprintf(buffer, 128, "HTTP/1.1 200 OK\r\nDate: %s, %02d %s %d %02d:%02d:%02d GMT\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n",
		 week_days[now_tm->tm_wday], now_tm->tm_mday, months[now_tm->tm_mon], now_tm->tm_year + 1900, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
	write(connfd, buffer, strlen(buffer));

	inform_queue_send(&queue, INFORM_TYPE_CONNECTION_REQUEST);
	
        close(connfd);
        sleep(1);
     }

    printf("ret=%d\n", ret);
    pthread_exit(NULL);
}

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
