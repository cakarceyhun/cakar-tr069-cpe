#include "connection.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SHOULD_NOT_BE_HERE assert(0);

#define PORT 7547

int connection_init(struct connection_s* connection, char* ip, int port)
{
    struct sockaddr_in serv_addr;
    if ((connection->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(connection->sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    return 0;
}

int connection_send(struct connection_s* connection, char* buffer, int length)
{
    send(connection->sock, buffer, length, 0 );

    return 0;
}

int connection_receive(struct connection_s* connection, char* buffer, int length)
{
    int valread;

    valread = read(connection->sock, buffer, length);

    if(valread != length) {
        printf("all data cannot be read\n");
        return -1;
    }

    return 1;
}

int connection_close(struct connection_s* connection)
{
    return 0;
}
