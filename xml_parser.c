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

struct XmlParserResult
{
    int id;
    const char *path;
    char *value;
    bool found;
    struct XmlParserResult *next;
};

struct XmlParserResults
{
    struct XmlParserResult *first;
    struct XmlParserResult *end;
};

struct XmlParser
{
    char *line;
    size_t length;
    char *untag_line;
    size_t untag_length;
    char *value;
    size_t value_length;
    char previous;
    enum state_e state;
    struct XmlParserResults results;
};

void* parse_xml_init(void)
{
    struct XmlParser *parser = malloc(sizeof(struct XmlParser));
    
    parser->line = malloc(4000);
    parser->length = 0;
    parser->untag_line = malloc(4000);
    parser->untag_length = 0;
    parser->value = malloc(4000);
    parser->value_length = 0;
    parser->previous = '\0';
    parser->state = OUTSIDE_OF_TAG;
    parser->results.first = NULL;
    parser->results.end = NULL;

    return (void *) parser;
}

int parse_xml_register(void* parser_vp, const char *path)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);

    struct XmlParserResult *item = calloc(1, sizeof(struct XmlParserResult));
    if (!item) {
	printf("SHOULD NOT BE HERE\n");
	exit(1);
    }

    item->path = path;

    if (!parser->results.end) {
	item->id = 0;

	parser->results.first = item;
	parser->results.end = item;
    } else {
	item->id = parser->results.end->id + 1;

	parser->results.end->next = item;
	parser->results.end->next = parser->results.end;
    }

    return item->id; 
}

static void parse_xml_execute(void* parser_vp, const char *path, const char *value)
{
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);
    struct XmlParserResult *item = parser->results.first;

    while (item) {
	if (strcmp(path, item->path) == 0) {
	    item->value = strdup(value);
	    item->found = true;
	}

	item = item->next;
    }
}

bool parse_xml_result_exists(void* parser_vp, int id)
{
    int i;
    struct XmlParser* parser = *((struct XmlParser**) parser_vp);
    struct XmlParserResult *item = parser->results.first;
    
    for (i = 0; item; i++) {
	item = item->next;
    }

    if (item) {
	return i;
    } else {
	return -1;
    }
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
	free(parser->line);
	free(parser->untag_line);
	free(parser->value);

	struct XmlParserResult *item = parser->results.first;
	while (item) {
	    struct XmlParserResult *next = item->next;

	    if (item->value) {
		free(item->value);
	    }
	    free(item);
	    item = next;
	}

	*((struct XmlParser**) parser_vp) = NULL;
    }
}
