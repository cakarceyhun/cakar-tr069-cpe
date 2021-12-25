#include "datamodel.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "external/sqlite-3.36.0/sqlite3.h"

#define DATABASE_FILE "/tmp/cpe.db"

pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

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

struct sqlite3_handler_s
{
    sqlite3_stmt *stmt;
    sqlite3* db;
    struct sqlite3_handler_s *next;
};

struct gpv_handler_s
{
    struct sqlite3_handler_s *first;
    struct sqlite3_handler_s *last;
};

const char *get_parameter_values_string(void **handler_pv, const char *path)
{
    char query[1024];
    struct gpv_handler_s *handler;
    const char *result;

    pthread_mutex_lock(&db_lock);

    if (*handler_pv == NULL) {
        *((struct gpv_handler_s**) handler_pv) = calloc(1, sizeof(struct gpv_handler_s));
    }

    handler = *((struct gpv_handler_s**) handler_pv);
    if (!handler) {
        printf("memory allocation error\n");

        return NULL;
    }

    if (!handler->last) {
        handler->last = calloc(1, sizeof(struct sqlite3_handler_s));
        if (!handler->last) {
            printf("memory allocation error\n");

            return NULL;
        }

        handler->first = handler->last;
    } else {
        handler->last->next = calloc(1, sizeof(struct sqlite3_handler_s));
        if (!handler->last->next) {
            printf("memory allocation error\n");

            return NULL;
        }
        handler->last = handler->last->next;
    }

    if (!handler->last->db) {
        if (sqlite3_open(DATABASE_FILE, &handler->last->db) != SQLITE_OK) {
            printf("%s\n", sqlite3_errmsg(handler->last->db));
            close_get_parameter_values(handler_pv);

            return NULL;
        }
    }

    snprintf(query, 1024, "SELECT [value] FROM [paths] where [path]='%s'", path);
    if (sqlite3_prepare(handler->last->db, query, strlen(query), &handler->last->stmt, NULL) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(handler->last->db));
        close_get_parameter_values(handler_pv);

        return NULL;
    }

    if (sqlite3_step(handler->last->stmt) != SQLITE_ROW) {
        close_get_parameter_values(handler_pv);

        return NULL;
    }

    result = (const char *) sqlite3_column_text(handler->last->stmt, 0);

    pthread_mutex_unlock(&db_lock);

    return result;
}

void close_get_parameter_values(void **handler_pv)
{
    struct gpv_handler_s *handler = *((struct gpv_handler_s**) handler_pv);

    pthread_mutex_lock(&db_lock);
    if (handler) {
        struct sqlite3_handler_s *sqlite3_handler = (struct sqlite3_handler_s *) handler->first;
        while(sqlite3_handler) {
            struct sqlite3_handler_s *next = (struct sqlite3_handler_s *) sqlite3_handler->next;

            sqlite3_finalize(sqlite3_handler->stmt);
            sqlite3_close(sqlite3_handler->db);
            free(sqlite3_handler);
            sqlite3_handler = next;
        }
        free(handler);

        *((struct sqlite_handler_s**) handler_pv) = NULL;
    }
    pthread_mutex_unlock(&db_lock);
}

int get_forced_parameter_values_length(void)
{
    char *query;
    sqlite3_stmt* stmt;
    sqlite3* db;

    pthread_mutex_lock(&db_lock);
    
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

    pthread_mutex_unlock(&db_lock);

    return value;
}

struct parameter_handler
{
    sqlite3_stmt* stmt;
    sqlite3* db;
};

void* get_forced_parameter_values_start(void)
{
    struct parameter_handler* handler = calloc(1, sizeof(struct parameter_handler));
    char *query;

    pthread_mutex_lock(&db_lock);
    
    if (sqlite3_open(DATABASE_FILE, &handler->db) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(handler->db));

        free(handler);
        handler = NULL;

        goto fail;
    }

    query = "SELECT [path],[value] FROM [paths] where [is_forced]=1";
    if (sqlite3_prepare(handler->db, query, strlen(query), &handler->stmt, NULL) != SQLITE_OK) {
        printf("%s\n", sqlite3_errmsg(handler->db));

        goto fail;
    }

    goto success;

fail:
    if (handler) {
        if (handler->db) {
            sqlite3_close(handler->db);
        }
        free(handler);
        handler = NULL;
    }

success:
    pthread_mutex_unlock(&db_lock);

    return (void *) handler;
}

bool get_forced_parameter_values_next(void *handler_vp, const char** path, const char** value)
{
    bool ret = true;
    struct parameter_handler* handler = *((struct parameter_handler**) handler_vp);

    pthread_mutex_lock(&db_lock);
    
    if (sqlite3_step(handler->stmt) != SQLITE_ROW) {
        get_forced_parameter_values_end(handler_vp);

        ret = false;
        goto end;
    }

    *path = (const char *) sqlite3_column_text(handler->stmt, 0);
    *value = (const char *) sqlite3_column_text(handler->stmt, 1);

    pthread_mutex_unlock(&db_lock);

    return ret;
}

void get_forced_parameter_values_end(void *handler_vp)
{
    struct parameter_handler* handler = *((struct parameter_handler**) handler_vp);

    pthread_mutex_lock(&db_lock);
    if (handler) {
        sqlite3_finalize(handler->stmt);
        sqlite3_close(handler->db);

        free(handler);
        *((struct parameter_handler**) handler_vp) = NULL;
    }
    pthread_mutex_unlock(&db_lock);
}
