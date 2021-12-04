#ifndef DATAMODEL_H_CEYHUNCAKAR_2021_11_08
#define DATAMODEL_H_CEYHUNCAKAR_2021_11_08

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>

#define SHOULD_NOT_BE_HERE assert(0);

enum data_type_e
{
	TYPE_UNFOUND,
	TYPE_STRING,
	TYPE_INTEGER,
	TYPE_BOOLEAN
};

void create_database();

enum data_type_e get_parameter_type(const char* path);

int get_forced_parameter_values_length(void);
void* get_forced_parameter_values_start(void);
bool get_forced_parameter_values_next(void *handler, const char** path, const char** value);
void get_forced_parameter_values_end(void *handler);

void get_parameter_values_string(const char* path, char* output, size_t max_length);

void set_parameter_values_string(const char* path, const char* value);
void set_parameter_values_int(const char* path, int32_t value);
void set_parameter_values_bool(const char* path, bool value);

#endif
