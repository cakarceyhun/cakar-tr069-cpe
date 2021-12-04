#ifndef HTTP_GENERATOR_H_CEYHUNCAKAR_2021_11_27
#define HTTP_GENERATOR_H_CEYHUNCAKAR_2021_11_27

#include <stdlib.h>

void append_http_header(char* sendbuf, size_t length, char* host, char* session_id, size_t maximum_size);

#endif
