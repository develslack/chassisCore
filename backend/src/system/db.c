#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "db.h"
#include "db_schemas.h"
#include "db_catalog.h"

static MYSQL *db_conn = NULL;
static DB_Config db_config;
static DB_Schema db_schema;
static SP_Catalog db_sp_catalog;

int db_init(const char *env_path, const char *sp_catalog_path) {
    // 1. Cargar .env
    if (env_load_config(env_path, &db_config) != 0) return -1;

    // 2. Inicializar MySQL
    db_conn = mysql_init(NULL);
    if (!db_conn) return -1;

    // Activar soporte para múltiples resultados (necesario para Stored Procedures)
    unsigned long flags = CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS;

    if (!mysql_real_connect(db_conn, db_config.host, db_config.user, db_config.pass,
                            db_config.dbname, db_config.port, NULL, flags)) {
        fprintf(stderr, "[DB Engine] Connection failed: %s\n", mysql_error(db_conn));
        return -1;
    }

    // 3. Inspeccionar esquema
    if (db_schema_build(db_conn, db_config.dbname, db_config.prefix, &db_schema) != 0) return -1;

    // 4. Cargar catálogo de SP
    if (db_catalog_init(sp_catalog_path, &db_sp_catalog) != 0) return -1;

    printf("[DB Engine] Motor iniciado exitosamente en modo BLACKBOX.\n");
    return 0;
}

int db_call_sp(const char *sp_key, const char **params, int param_count, DB_DataSet *out_dataset) {
    if (!db_conn || !out_dataset) return -1;

    memset(out_dataset, 0, sizeof(DB_DataSet));

    // 1. Resolver el SP real desde el catálogo
    const char *real_sp = db_catalog_get(&db_sp_catalog, sp_key);
    if (!real_sp) {
        snprintf(out_dataset->error_msg, sizeof(out_dataset->error_msg),
                 "SP Key '%s' no encontrada en el catálogo", sp_key);
        out_dataset->status = -1;
        return -1;
    }

    // 2. Armar la consulta SQL: CALL real_sp('p1', 'p2', ...)
    char query[2048];
    int offset = snprintf(query, sizeof(query), "CALL %s(", real_sp);

    for (int i = 0; i < param_count; i++) {
        char escaped[512];
        mysql_real_escape_string(db_conn, escaped, params[i] ? params[i] : "",
                                 strlen(params[i] ? params[i] : ""));

        offset += snprintf(query + offset, sizeof(query) - offset,
                           "'%s'%s", escaped, (i < param_count - 1) ? ", " : "");
    }
    snprintf(query + offset, sizeof(query) - offset, ")");

    // 3. Ejecutar consulta
    if (mysql_query(db_conn, query)) {
        strncpy(out_dataset->error_msg, mysql_error(db_conn), sizeof(out_dataset->error_msg) - 1);
        out_dataset->status = -1;
        return -1;
    }

    out_dataset->affected_rows = (int)mysql_affected_rows(db_conn);
    out_dataset->last_insert_id = mysql_insert_id(db_conn);

    // 4. Procesar conjunto de resultados
    MYSQL_RES *res = mysql_store_result(db_conn);
    if (res) {
        out_dataset->num_rows = (int)mysql_num_rows(res);
        out_dataset->num_cols = (int)mysql_num_fields(res);

        // A. Guardar nombres de columnas (Headers)
        out_dataset->headers = malloc(sizeof(char*) * out_dataset->num_cols);
        MYSQL_FIELD *fields = mysql_fetch_fields(res);
        for (int i = 0; i < out_dataset->num_cols; i++) {
            out_dataset->headers[i] = strdup(fields[i].name);
        }

        // B. Guardar matriz de datos
        out_dataset->data = malloc(sizeof(char**) * out_dataset->num_rows);
        MYSQL_ROW row;
        int r = 0;

        while ((row = mysql_fetch_row(res))) {
            out_dataset->data[r] = malloc(sizeof(char*) * out_dataset->num_cols);
            for (int c = 0; c < out_dataset->num_cols; c++) {
                out_dataset->data[r][c] = strdup(row[c] ? row[c] : "");
            }
            r++;
        }

        mysql_free_result(res);
    }

    // Limpiar paquetes remanentes de Stored Procedures
    while (mysql_next_result(db_conn) == 0) {
        MYSQL_RES *extra = mysql_store_result(db_conn);
        if (extra) mysql_free_result(extra);
    }

    out_dataset->status = 0;
    return 0;
}

void db_free_dataset(DB_DataSet *ds) {
    if (!ds) return;

    if (ds->headers) {
        for (int i = 0; i < ds->num_cols; i++) {
            free(ds->headers[i]);
        }
        free(ds->headers);
    }

    if (ds->data) {
        for (int r = 0; r < ds->num_rows; r++) {
            for (int c = 0; c < ds->num_cols; c++) {
                free(ds->data[r][c]);
            }
            free(ds->data[r]);
        }
        free(ds->data);
    }

    memset(ds, 0, sizeof(DB_DataSet));
}

void db_close() {
    if (db_conn) {
        mysql_close(db_conn);
        db_conn = NULL;
    }
}
