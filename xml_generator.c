#include <time.h>
#include "datamodel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *soap_inform_header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<soap-env:Envelope xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:soap-env=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\r\n\
<soap-env:Header>\r\n\
    <cwmp:ID soap-env:mustUnderstand=\"1\">5sf6q47c</cwmp:ID>\r\n\
</soap-env:Header>\r\n\
<soap-env:Body>\r\n\
    <cwmp:Inform>\r\n\
        <DeviceId>\r\n\
            <Manufacturer>%s</Manufacturer>\r\n\
            <OUI>%s</OUI>\r\n\
            <ProductClass>%s</ProductClass>\r\n\
            <SerialNumber>%s</SerialNumber>\r\n\
        </DeviceId>\r\n\
        <Event soap-enc:arrayType=\"cwmp:EventStruct[1]\">\r\n\
            <EventStruct>\r\n\
                <EventCode>%s</EventCode>\r\n\
                <CommandKey/>\r\n\
            </EventStruct>\r\n\
        </Event>\r\n\
        <MaxEnvelopes>1</MaxEnvelopes>\r\n\
        <CurrentTime>%s</CurrentTime>\r\n\
        <RetryCount>0</RetryCount>\r\n\
        <ParameterList soap-enc:arrayType=\"cwmp:ParameterValueStruct[%d]\">\r\n\
";

const char *soap_inform_parameter_value_struct =
"           <ParameterValueStruct>\r\n\
                <Name>%s</Name><Value xsi:type=\"xsd:string\">%s</Value>\r\n\
            </ParameterValueStruct>\r\n\
";

const char *soap_inform_footer = "\
        </ParameterList>\r\n\
    </cwmp:Inform>\r\n\
</soap-env:Body>\r\n\
</soap-env:Envelope>\r\n";

char *xml_generator_create(const char *type)
{
    char *result;
    time_t now = time(NULL);
    struct tm *now_tm = gmtime(&now);
    char time_str[32];
    const char* path, *value;
    const char *manufacturer;
    const char *oui;
    const char *product_class, *serial_number;
    int number_of_forced_parameters = 0;
    void *handler = NULL;
    size_t length = 0;

    manufacturer = get_parameter_values_string(&handler, "Device.DeviceInfo.Manufacturer");
    oui = get_parameter_values_string(&handler, "Device.DeviceInfo.OUI");
    product_class = get_parameter_values_string(&handler, "Device.DeviceInfo.ProductClass");
    serial_number = get_parameter_values_string(&handler, "Device.DeviceInfo.SerialNumber");
    snprintf(time_str, 32, "%d-%02d-%02dT%02d:%02d:%02d.000Z",
	     now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday,
	     now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
    number_of_forced_parameters = get_forced_parameter_values_length();

    length += strlen(soap_inform_header);
    length += strlen(manufacturer);
    length += strlen(oui);
    length += strlen(product_class);
    length += strlen(time_str);
    length += strlen(serial_number);

    void *forced_handler = get_forced_parameter_values_start();
    while (get_forced_parameter_values_next(&forced_handler, &path, &value)) {
	length += strlen(soap_inform_parameter_value_struct) + strlen(path) + strlen(value);
    }
    get_forced_parameter_values_end(&forced_handler);

    length += strlen(soap_inform_footer);

    result = calloc(1, length + 1024);
    if (!result) {
	return NULL;
    }

    sprintf(result, soap_inform_header, manufacturer, oui, product_class, serial_number, type,
	    time_str, number_of_forced_parameters);
    forced_handler = get_forced_parameter_values_start();
    while (get_forced_parameter_values_next(&forced_handler, &path, &value)) {
        sprintf(&result[strlen(result)], soap_inform_parameter_value_struct, path, value);
    }
    get_forced_parameter_values_end(&forced_handler);

    strcat(result, soap_inform_footer);

    close_get_parameter_values(&handler);

    //printf("%s\n", result);

    return result;
}
