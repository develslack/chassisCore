#ifndef DB_H
#define DB_H

#include "env_loader.h"

// Estructura genérica que encapsula cualquier respuesta en matriz de strings
typedef struct {
    int num_rows;
    int num_cols;
    char **headers;      // Nombres de las columnas [num_cols]
    char ***data;        // Matriz de valores data[num_rows][num_cols]
    int affected_rows;
    long long last_insert_id;
    int status;          // 0 = OK, != 0 = Error
    char error_msg[256];
} DB_DataSet;

// Inicializa el motor leyendo .env, creando la conexión, escaneando el esquema y cargando SPs
int db_init(const char *env_path, const char *sp_catalog_path);

// Función universal Blackbox para llamar a cualquier Stored Procedure
int db_call_sp(const char *sp_key, const char **params, int param_count, DB_DataSet *out_dataset);

// Libera la memoria del DataSet genérico
void db_free_dataset(DB_DataSet *ds);

// Cierra la conexión y libera los recursos del motor
void db_close();

#endif
