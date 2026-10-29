#ifndef DB_SCHEMA_H
#define DB_SCHEMA_H

#include <mysql/mysql.h>

#define MAX_TABLES 128

typedef struct {
    char logical_name[64];  // Sin prefijo (ej: "users")
    char physical_name[64]; // Con prefijo (ej: "mc_users")
} Table_Mapping;

typedef struct {
    Table_Mapping tables[MAX_TABLES];
    int count;
    char prefix[32];
} DB_Schema;

int db_schema_build(MYSQL *conn, const char *dbname, const char *prefix, DB_Schema *schema);
const char* db_schema_get_table(const DB_Schema *schema, const char *logical_name);

#endif
