#include <time.h>
#include "datamodel.h"
#include <stdio.h>
#include <string.h>

void xml_generator_create(char* sendbuf, const char *type)
{
    time_t now = time(NULL);
    struct tm *now_tm;
    char time_str[32];
    char buffer[128];

    sendbuf[0] = '\0';
    now_tm = gmtime(&now);

    snprintf(time_str, 32, "%d-%02d-%02dT%02d:%02d:%02d.000Z", now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);

    strcat(sendbuf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
    strcat(sendbuf, "<soap-env:Envelope xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:soap-env=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\r\n");
    strcat(sendbuf, "<soap-env:Header>\r\n");
    strcat(sendbuf, "\t<cwmp:ID soap-env:mustUnderstand=\"1\">5sf6q47c</cwmp:ID>\r\n");
    strcat(sendbuf, "</soap-env:Header>\r\n");
    strcat(sendbuf, "<soap-env:Body>\r\n");
    strcat(sendbuf, "\t<cwmp:Inform>\r\n");
    strcat(sendbuf, "\t\t<DeviceId>\r\n");

    strcat(sendbuf, "\t\t\t<Manufacturer>");
    get_parameter_values_string("Device.DeviceInfo.Manufacturer", &sendbuf[strlen(sendbuf)], 1024);
    strcat(sendbuf, "</Manufacturer>\r\n");

    strcat(sendbuf, "\t\t\t<OUI>");
    get_parameter_values_string("Device.DeviceInfo.OUI", &sendbuf[strlen(sendbuf)], 1024);
    strcat(sendbuf, "</OUI>\r\n");

    strcat(sendbuf, "\t\t\t<ProductClass>");
    get_parameter_values_string("Device.DeviceInfo.ProductClass", &sendbuf[strlen(sendbuf)], 1024);
    strcat(sendbuf, "</ProductClass>\r\n");

    strcat(sendbuf, "\t\t\t<SerialNumber>");
    get_parameter_values_string("Device.DeviceInfo.SerialNumber", &sendbuf[strlen(sendbuf)], 1024);
    strcat(sendbuf, "</SerialNumber>\r\n");

    strcat(sendbuf, "\t\t</DeviceId>\r\n");

    strcat(sendbuf, "\t\t<Event soap-enc:arrayType=\"cwmp:EventStruct[1]\">\r\n");
    strcat(sendbuf, "\t\t\t<EventStruct>\r\n");

    strcat(sendbuf, "\t\t\t\t<EventCode>");
    strcat(sendbuf, type);
    strcat(sendbuf, "</EventCode>\r\n");

    strcat(sendbuf, "\t\t\t\t<CommandKey/>\r\n");
    strcat(sendbuf, "\t\t\t</EventStruct>\r\n");
    strcat(sendbuf, "\t\t</Event>\r\n");
    strcat(sendbuf, "\t\t<MaxEnvelopes>1</MaxEnvelopes>\r\n");

    strcat(sendbuf, "\t\t<CurrentTime>");
    strcat(sendbuf, time_str);
    strcat(sendbuf, "</CurrentTime>\r\n");

    strcat(sendbuf, "\t\t<RetryCount>0</RetryCount>\r\n");

    const char* path, * value;

    snprintf(buffer, 128, "\t\t<ParameterList soap-enc:arrayType=\"cwmp:ParameterValueStruct[%d]\">\r\n", (int)get_forced_parameter_values_length());
    strcat(sendbuf, buffer);

    void *handler = get_forced_parameter_values_start();
    while (get_forced_parameter_values_next(&handler, &path, &value)) {
        strcat(sendbuf, "\t\t\t<ParameterValueStruct>\r\n");
        strcat(sendbuf, "\t\t\t\t<Name>");
        strcat(sendbuf, path);
        strcat(sendbuf, "</Name>\r\n");
        strcat(sendbuf, "\t\t\t\t<Value xsi:type=\"xsd:string\">");
        strcat(sendbuf, value);
        strcat(sendbuf, "</Value>\r\n");
        strcat(sendbuf, "\t\t\t</ParameterValueStruct>\r\n");
    }

    get_forced_parameter_values_end(&handler);
    strcat(sendbuf, "\t\t</ParameterList>\r\n");

    strcat(sendbuf, "</cwmp:Inform>\r\n");
    strcat(sendbuf, "</soap-env:Body>\r\n");
    strcat(sendbuf, "</soap-env:Envelope>\r\n");
}
