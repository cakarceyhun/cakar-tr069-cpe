#include <stdio.h>
#include <assert.h>

#include "../xml_parser.c"

int main()
{
    FILE *f;

    f = fopen("GetParameterNames.xml", "rb");
    if (!f) {
        assert(0);
    }
    
    enum XmlParserState xml_state = XML_STATE_VALID;
    void* xml_parser = parse_xml_init();
    int inform_response_id = parse_xml_register(&xml_parser, "/soap-env:Envelope/soap-env:Body/cwmp:InformResponse");
    int get_parameter_names_id = parse_xml_register(&xml_parser, "/soap-env:Envelope/soap-env:Body/cwmp:GetParameterNames/ParameterPath");
    while (1) {
        char value[1];
        
        value[0] = fgetc(f);

        if (feof(f)) {
            break;
        }
        
        xml_state = parse_xml_push(&xml_parser, value[0]);
        if (xml_state == XML_STATE_INVALID) {
            break;
        }
    }

    
    printf("Hello World\n");

    return 0;
}
