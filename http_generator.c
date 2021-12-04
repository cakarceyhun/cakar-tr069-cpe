#include "http_generator.h"
#include <stdio.h>
#include <string.h>

void append_http_header(char* sendbuf, size_t length, char* host, char* session_id, size_t maximum_size)
{
    char buffer[1025] = "";
    (void)maximum_size; //TODO

    sendbuf[0] = '\0';
    strcat(sendbuf, "POST / HTTP/1.1\r\n");

    snprintf(buffer, 1025, "Content-Length: %zu\r\n", length);
    strcat(sendbuf, buffer);

    strcat(sendbuf, "Content-Type: text/xml; charset=\"utf-8\"\r\n");
    //strcat(sendbuf, "Authorization: Basic OEtBOFdBMTE1MTEwMDA0Mzo =\r\n");

    if (session_id) {
        snprintf(buffer, 1025, "Cookie: session=%s\r\n", session_id);
        strcat(sendbuf, buffer);
    }
    strcat(sendbuf, "Host: ");
    strcat(sendbuf, host);
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "Connection: keep-alive\r\n");
    strcat(sendbuf, "\r\n");
}
