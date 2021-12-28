#include "xml_parser.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

enum state_e {
    OUTSIDE_OF_TAG,
    TAG_NAME,
    TAG_IGNORE,
    UNTAG
};

struct XmlParserSearchItem
{
    int id;
    const char *path;
    bool found;
    struct XmlParserSearchItem *next;
};

struct XmlParserSearchItems
{
    struct XmlParserSearchItem *first;
    struct XmlParserSearchItem *end;
};

struct XmlParserResults
{
    struct XmlParserResult *first;
    struct XmlParserResult *end;
};


struct XmlParser
{
    char line[4000];
    size_t length;
    char untag_line[4000];
    size_t untag_length;
    char value[4000];
    size_t value_length;
    char previous;
    enum state_e state;
    struct XmlParserSearchItems search_items;
    struct XmlParserResults results;
};

void* parse_xml_init(void)
{
    struct XmlParser *parser = malloc(sizeof(struct XmlParser));
    
    parser->line[0] = '\0';
    parser->length = 0;
    parser->untag_line[0] = '\0';
    parser->untag_length = 0;
    parser->value[0] = '\0';
    parser->value_length = 0;
    parser->previous = '\0';
    parser->state = OUTSIDE_OF_TAG;
    parser->results.first = NULL;
    parser->results.end = NULL;
    parser->search_items.first = NULL;
    parser->search_items.end = NULL;
    
    return (void *) parser;
}

int parse_xml_register(void* parser_vp, const char *path)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);

    struct XmlParserSearchItem *item = calloc(1, sizeof(struct XmlParserSearchItem));
    if (!item) {
        printf("SHOULD NOT BE HERE\n");
        exit(1);
    }

    item->path = path;

    if (!parser->search_items.end) {
        item->id = 0;

        parser->search_items.first = item;
        parser->search_items.end = item;
    } else {
        item->id = parser->search_items.end->id + 1;

        parser->search_items.end->next = item;
        parser->search_items.end = parser->search_items.end->next;
    }

    return item->id;
}

struct XmlParserResult *parse_xml_get_results(void* parser_vp)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);

    return parser->results.first;
}

static void parse_xml_execute(void* parser_vp, const char *path, const char *value)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);
    struct XmlParserSearchItem *item = parser->search_items.first;

    while (item) {
        if (strcmp(path, item->path) == 0) {
            struct XmlParserResult *result;

            result = calloc(1, sizeof(struct XmlParserResult));
            result->value = strdup(value);
            result->path = item->path;
            result->id = item->id;
            
            if (!parser->results.end) {
                parser->results.first = result;
                parser->results.end = result;
            } else {
                parser->results.end->next = result;
                parser->results.end = parser->results.end->next;
            }

            item->found = true;
        }

        item = item->next;
    }
}

bool parse_xml_result_exists(void* parser_vp, int id)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);
    struct XmlParserResult *result = parser->results.first;
    
    while (result) {
        if (result->id == id) {
            return true;
        }
        
        result = result->next;
    }

    return false;
}

enum XmlParserState parse_xml_push(void* parser_vp, char v)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);

    if (parser->state == OUTSIDE_OF_TAG) {
	if (v == '<') {
	    parser->state = TAG_NAME;
	    parser->line[parser->length++] = '/';
	    parser->previous = '<';
	}
	else {
	    parser->value[parser->value_length++] = v;
	    parser->previous = '\0';
	}
    } else if (parser->state == TAG_NAME) {
	if (v == ' ' || v == '\t' || v == '\r' || v == '\n') {
	    parser->state = TAG_IGNORE;
	    parser->previous = '\0';
	} else if (parser->previous == '<' && v == '/') {
	    parser->state = UNTAG;
	    parser->previous = '\0';
	} else if (v == '/') {
	    parser->previous = '/';
        } else if (parser->previous == '/' && v == '>') {
	    parser->state = OUTSIDE_OF_TAG;
	    parser->value_length = 0;
	    parser->previous = '\0';
	} else if (v == '>') {
	    parser->state = OUTSIDE_OF_TAG;
	    parser->value_length = 0;
	    parser->previous = '\0';
       } else {
	    parser->line[parser->length++] = v;
	    parser->previous = '\0';
	}
    } else if (parser->state == TAG_IGNORE) {
	if (v == '/' || v == '?') {
	    parser->previous = v;
	} else if ((parser->previous == '/' || parser->previous == '?') && v == '>') {
	    size_t i;
	    parser->state = OUTSIDE_OF_TAG;

	    for (i = 0; i < parser->length; ++i) {
		if (parser->line[parser->length - 1 - i] == '/') {
		    parser->line[parser->length - 1 - i] = '\0';
		    parser->length -= (i + 1);
		    break;
		}
	    }  
	    parser->value_length = 0;
	    parser->previous = '\0';
	} else if (v == '>') {
	    parser->state = OUTSIDE_OF_TAG;
	    parser->value_length = 0;
	    parser->previous = '\0';
	} else {
	    parser->previous = '\0';
	}
    } else if (parser->state == UNTAG) {
	if (v == ' ' || v == '\t' || v == '\r' || v == '\n') {
	    parser->state = TAG_IGNORE;
	    parser->previous = '\0';
	} else if (v == '>') {
	    parser->state = OUTSIDE_OF_TAG;
	    parser->value[parser->value_length] = '\0';
	    if (parser->line[parser->length - 1] == '/') {
		parser->line[parser->length - 1] = '\0';
	    } else {
		parser->line[parser->length] = '\0';
	    }

	    parser->untag_line[parser->untag_length] = '\0';

	    parse_xml_execute(parser_vp, parser->line, parser->value);
	    //printf("%s=%s\n", parser->line, parser->value);
	    size_t start = strlen(parser->line) - strlen(parser->untag_line);
	    const char* last = &parser->line[start];
	    if (strcmp(parser->untag_line, last) != 0) {
		return XML_STATE_INVALID;
	    }
	    parser->line[start - 1] = '\0';
	    parser->length = start - 1;
	    parser->previous = '\0';
	    parser->value_length = 0;
	    parser->untag_length = 0;
	} else {
	    parser->untag_line[parser->untag_length++] = v;
	    parser->previous = '\0';
	}
    }

    return XML_STATE_VALID;
}

void parse_xml_close(void* parser_vp)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);

    if (parser) {
        struct XmlParserSearchItem *item = parser->search_items.first;
        while (item) {
            struct XmlParserSearchItem *next = item->next;

            free(item);
            item = next;
        }

        struct XmlParserResult *result = parser->results.first;
        while (result) {
            struct XmlParserResult *next = result->next;

            if (result->value) {
                free(result->value);
            }
            free(result);
            result = next;
        }

        free(parser);

        *((struct XmlParser**) parser_vp) = NULL;
    }
}
