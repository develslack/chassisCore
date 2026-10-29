#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "db_schemas.h"

int db_schema_build(MYSQL *conn, const char *dbname, const char *prefix, DB_Schema *schema) {
    schema->count = 0;
    strncpy(schema->prefix, prefix, sizeof(schema->prefix) - 1);

    char query[256];
    snprintf(query, sizeof(query),
             "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = '%s'", dbname);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "[SCHEMA] Error al inspeccionar esquema: %s\n", mysql_error(conn));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) return -1;

    MYSQL_ROW row;
    size_t prefix_len = strlen(prefix);

    while ((row = mysql_fetch_row(res)) && schema->count < MAX_TABLES) {
        const char *phys_name = row[0];
        strncpy(schema->tables[schema->count].physical_name, phys_name, 63);

        // Si la tabla tiene el prefijo, se lo extraemos para el nombre lógico
        if (prefix_len > 0 && strncmp(phys_name, prefix, prefix_len) == 0) {
            strncpy(schema->tables[schema->count].logical_name, phys_name + prefix_len, 63);
        } else {
            strncpy(schema->tables[schema->count].logical_name, phys_name, 63);
        }

        schema->count++;
    }

    mysql_free_result(res);
    printf("[SCHEMA] Introspección completada. %d tablas registradas.\n", schema->count);
    return 0;
}

const char* db_schema_get_table(const DB_Schema *schema, const char *logical_name) {
    for (int i = 0; i < schema->count; i++) {
        if (strcmp(schema->tables[i].logical_name, logical_name) == 0) {
            return schema->tables[i].physical_name;
        }
    }
    return logical_name; // Fallback al original si no se encuentra
}
