#ifndef HTTP_PARSER_H_CEYHUNCAKAR_2021_11_27
#define HTTP_PARSER_H_CEYHUNCAKAR_2021_11_27

#include <stddef.h>

enum HttpParserState
{
    STATE_VALID,
    STATE_INVALID,
    STATE_COMPLETED
};

struct HttpParser
{
    size_t content_length;
    int response;

    char line[1024];
    int position;
    char session_id[1024];
};

void parse_http_init(struct HttpParser* parser);
enum HttpParserState parse_http_push(struct HttpParser* parser, char value);
int parse_http_get_response(struct HttpParser* parser);
void parse_http_close(struct HttpParser* parser);

#endif

