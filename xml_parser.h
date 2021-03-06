#ifndef XML_PARSER_H_CEYHUNCAKAR_2021_12_05
#define XML_PARSER_H_CEYHUNCAKAR_2021_12_05

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

enum XmlParserState
{
    XML_STATE_VALID,
    XML_STATE_INVALID,
};

struct XmlParserResult
{
    int id;
    const char *path;
    char *value;
    struct XmlParserResult *next;
};

void *parse_xml_init(void);
bool parse_xml_result_exists(void* parser_vp, int id);
struct XmlParserResult *parse_xml_get_results(void* parser_vp);
int parse_xml_register(void* parser_vp, const char *path);
enum XmlParserState parse_xml_push(void* parser, char v);
void parse_xml_close(void *parser_vp);

#endif

