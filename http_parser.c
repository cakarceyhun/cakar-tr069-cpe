#include "http_parser.h"
#include <assert.h>
#include <string.h>
#include "common.h"

#define SHOULD_NOT_BE_HERE assert(0);

void parse_http_init(struct HttpParser* parser)
{
    if (!parser) {
        SHOULD_NOT_BE_HERE
    }

    parser->content_length = 0;
    parser->line[0] = '\0';
    parser->response = -1;
    parser->position = 0;
    parser->session_id[0] = '\0';
}

enum HttpParserState parse_http_push(struct HttpParser* parser, char value)
{
    if (parser->position == 1023) {
        SHOULD_NOT_BE_HERE
            return STATE_INVALID;
    }

    parser->line[parser->position++] = value;

    if (value == '\n') {
        parser->line[parser->position] = '\0';
        if (strstr(parser->line, "HTTP/1.1") == parser->line) {
            int result = extract_string(&parser->line[8], strlen(parser->line));
            if (result != -1) {
                parser->response = result;
            }
        }
        else if (strstr(parser->line, "Content-Length:") == parser->line) {
            int result = extract_string(&parser->line[15], strlen(parser->line));
            if (result != -1) {
                parser->content_length = result;
            }
        }
        else if (strstr(parser->line, "Set-Cookie:") == parser->line) {
            char* start = strstr(parser->line, "session=");
            if (start) {
                start += 8;

                memcpy(parser->session_id, start, strlen(start) - 2);
                parser->session_id[strlen(start) - 2] = '\0';
            }
        }
        else if (strstr(parser->line, "\r\n") == parser->line) {
            return STATE_COMPLETED;
        }
        parser->position = 0;
    }

    return STATE_VALID;
}