#include "datamodel.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "external/sqlite-3.36.0/sqlite3.h"

#define DATABASE_FILE "/tmp/cpe.db"

void exec(sqlite3* db, const char* sql)
{
    sqlite3_stmt* stmt;
    if (sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_step(stmt);
    }
    else
    {
        printf("%s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}


void create_database()
{
    sqlite3* db;

    if(access(DATABASE_FILE, F_OK ) == 0 ) {
	return;
    }
   
    if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
	return;
    }

    
    exec(db, "CREATE TABLE [paths]([id] INTEGER PRIMARY KEY ASC AUTOINCREMENT, [path], [value], [type], [is_forced] INTEGER)");

    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.OUI', '202BC1', 's', 0)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.ProductClass', 'BM632w', 's', 0)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.Manufacturer', 'Cakar', 's', 0)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.SerialNumber', 'SN00000001', 's', 0)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.SpecVersion', '1', 's', 1)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.HardwareVersion', '40501', 's', 1)");	
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.DeviceInfo.ProvisioningCode', '', 's', 1)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.ManagementServer.URL', 'http://127.0.0.1:7547/', 's', 1)");
    exec(db, "INSERT INTO [paths] ([path], [value], [type], [is_forced]) VALUES ('Device.ManagementServer.ConnectionRequestURL', 'http://localhost:8080/', 's', 1)");

    sqlite3_close(db);
}

void get_parameter_values_string(const char *path, char *output, size_t max_length)
{
    (void) output;
    (void) max_length;
    char query[1024];
    sqlite3_stmt* stmt;
    sqlite3* db;
    
    if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
	return;
    }

    snprintf(query, 1024, "SELECT [value] FROM [paths] where [path]='%s'", path);
    if (sqlite3_prepare(db, query, strlen(query), &stmt, NULL) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
	sqlite3_close(db);

	return;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return;
    }

    const unsigned char* value = sqlite3_column_text(stmt, 0);
    size_t length = strlen(value);
    
    if (length >= max_length) {
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return;
    }

    memcpy(output, value, length);
    output[length] = '\0';

    sqlite3_finalize(stmt);
    sqlite3_close(db);  
}

int get_forced_parameter_values_length(void)
{
    char *query;
    sqlite3_stmt* stmt;
    sqlite3* db;
    
    if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
	return -1;
    }

    query = "SELECT count([path]) FROM [paths] where [is_forced]=1";
    if (sqlite3_prepare(db, query, strlen(query), &stmt, NULL) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(db));
	sqlite3_close(db);

	return -1;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return -1;
    }

    int value = sqlite3_column_int(stmt, 0);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);  

    return value;
}

struct parameter_handler
{
    sqlite3_stmt* stmt;
    sqlite3* db;
};

void* get_forced_parameter_values_start(void)
{
    struct parameter_handler* handler = malloc(sizeof(struct parameter_handler)); 
    char *query;
    
    if (sqlite3_open(DATABASE_FILE, &handler->db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(handler->db));

	free(handler);
	handler = NULL;
	return NULL;
    }

    query = "SELECT [path],[value] FROM [paths] where [is_forced]=1";
    if (sqlite3_prepare(handler->db, query, strlen(query), &handler->stmt, NULL) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(handler->db));

	sqlite3_close(handler->db);
	free(handler);
	handler = NULL;
	
	return NULL;
    }

    return (void *) handler;
}

bool get_forced_parameter_values_next(void *handler_vp, const char** path, const char** value)
{
    struct parameter_handler* handler = *((struct parameter_handler**) handler_vp);
    
    if (sqlite3_step(handler->stmt) != SQLITE_ROW) {
	get_forced_parameter_values_end(handler_vp);

	return false;
    }

    *path = sqlite3_column_text(handler->stmt, 0);
    *value = sqlite3_column_text(handler->stmt, 0);

    return true;
}

void get_forced_parameter_values_end(void *handler_vp)
{
    struct parameter_handler* handler = *((struct parameter_handler**) handler_vp);
    
    if (handler) {
	sqlite3_finalize(handler->stmt);
	sqlite3_close(handler->db);

	free(handler);
	*((struct parameter_handler**) handler_vp) = NULL;
    }
}
