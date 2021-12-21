#include <string.h>
#include <stdio.h>

#include "inform.h"

#include "http_generator.h"
#include "xml_generator.h"
#include "http_parser.h"
#include "xml_parser.h"

#include "datamodel.h"
#include "common.h"
#include "connection.h"

#define DEFAULT_PORT 7547
#define MAXIMUM_BUFFER_LENGTH (10 * 1024)

enum Mode
{
    MODE_INFORM_INIT,
    MODE_EMPTY,
};

void inform(enum inform_type_e type)
{
    struct connection_s connection;
    char sendbuf[MAXIMUM_BUFFER_LENGTH] = "";
    char httpheader[1024] = "";
    char host[1024] = "";
    int iResult;
    size_t j;
    enum Mode mode = MODE_INFORM_INIT;
    struct HttpParser parser;
    int port = -1;
 
    get_parameter_values_string("Device.ManagementServer.URL", host, sizeof(host) - 1);
    char *start = strstr(host, "/");
    if (!start) {
	assert(0);
    }
    start += 2;
    char *end = strchr(start, ':');
    if (end) {
	end[0] = '\0';
	port = extract_string(end + 1, strlen(end + 1));
    } else {
	port = DEFAULT_PORT;
    }
    
    connection_init(&connection, start, port);

    while (1) {
	enum HttpParserState state = STATE_VALID;
	
	sendbuf[0] = '\0';

	if (mode == MODE_INFORM_INIT) {
	    const char *inform_type_text =
		(type == INFORM_TYPE_BOOTSTRAP) ? "0 BOOTSTRAP" :
		(type == INFORM_TYPE_BOOT) ? "1 BOOT" :
		(type == INFORM_TYPE_PERIODIC) ? "2 PERIODIC" :
		(type == INFORM_TYPE_VALUE_CHANGE) ? "4 VALUE CHANGE" :
		(type == INFORM_TYPE_CONNECTION_REQUEST) ? "6 CONNECTION REQUEST" : "";
	  
	    xml_generator_create(sendbuf, inform_type_text);
	    append_http_header(httpheader, strlen(sendbuf), host, NULL, 1024);
	} else if (mode == MODE_EMPTY) {
	    append_http_header(httpheader, 0, host, parser.session_id, 1024);
	} else {
	    assert(0);
	}

	parse_http_init(&parser);

	connection_send(&connection, httpheader, (int)strlen(httpheader));
	if (strlen(sendbuf)) {
	    connection_send(&connection, sendbuf, (int)strlen(sendbuf));
	}

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

	if (state != STATE_COMPLETED) {
	    break;
	}

	int response = parse_http_get_response(&parser);
	parse_http_close(&parser);
	if (response != 200) {
	    break;
	}

	enum XmlParserState xml_state = XML_STATE_VALID;
	void* xml_parser = parse_xml_init();
	int inform_response_id = parse_xml_register(&xml_parser, "/soap-env:Envelope/soap-env:Body/cwmp:InformResponse");
	for (j = 0; j < parser.content_length; ++j) {
	    char value[1];

	    iResult = connection_receive(&connection, value, 1);
	    if (iResult == 0) {
		printf("Connection closed\n");
		break;
	    }

	    xml_state = parse_xml_push(&xml_parser, value[0]);
	    if (xml_state == XML_STATE_INVALID) {
		break;
	    }
	}

	if (parse_xml_result_exists(&xml_parser, inform_response_id)) {
	    mode = MODE_EMPTY;
	}

	parse_xml_close(&xml_parser);
    }

    connection_close(&connection);
}
