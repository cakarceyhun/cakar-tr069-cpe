#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "datamodel.h"

#include "http_generator.h"
#include "xml_inform_generator.h"
#include "http_parser.h"

#include "connection.h"
#include "datamodel.h"

#define SHOULD_NOT_BE_HERE assert(0);

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "7547"
#define MAXIMUM_BUFFER_LENGTH (10 * 1024)

int main(int argc, char** argv)
{
    struct connection_s connection;
    struct addrinfo* result = NULL, *ptr = NULL;
    char sendbuf[MAXIMUM_BUFFER_LENGTH] = "";
    char httpheader[1024] = "";
    char host[1024] = "";
    int iResult;

    create_database();

    sendbuf[0] = '\0';
    append_xml(sendbuf, MAXIMUM_BUFFER_LENGTH);

    get_parameter_values_string("Device.ManagementServer.URL", host, sizeof(host) - 1);
    append_http_header(httpheader, strlen(sendbuf), host, NULL, 1024);

    connection_init(&connection, "127.0.0.1", DEFAULT_PORT);

    connection_send(&connection, httpheader, (int)strlen(httpheader));
    connection_send(&connection, sendbuf, (int)strlen(sendbuf));

    enum HttpParserState state = STATE_VALID;
    struct HttpParser parser;
    parse_http_init(&parser);
    do {
        char value[1];

        iResult = connection_receive(&connection, value, 1);

        if (iResult == 0) {
            printf("Connection closed\n");
            break;
        }

        state = parse_http_push(&parser, value[0]);
        if (state == STATE_INVALID || state == STATE_COMPLETED) {
            break;
        }
    } while (iResult > 0);

    char* xml_buffer = calloc(1, parser.content_length + 2);
    if (!xml_buffer) {
        printf("memory allocation error\n");
        SHOULD_NOT_BE_HERE
        return -1;
    }


    iResult = connection_receive(&connection, xml_buffer, (int)parser.content_length);
    if (iResult == 0) {
        printf("Connection closed\n");
    }

    xml_buffer[parser.content_length] = '\n';

    printf("%s", xml_buffer);

    append_http_header(httpheader, 0, host, parser.session_id, 1024);
    connection_send(&connection, httpheader, (int)strlen(httpheader));

    state = STATE_VALID;
    parse_http_init(&parser);
    do {
        char value[1];
        iResult = connection_receive(&connection, value, 1);

        if (iResult == 0) {
            printf("Connection closed\n");
            break;
        }

        state = parse_http_push(&parser, value[0]);
        if (state == STATE_INVALID || state == STATE_COMPLETED) {
            break;
        }
    } while (iResult > 0);

    connection_close(&connection);

    return 0;
}
