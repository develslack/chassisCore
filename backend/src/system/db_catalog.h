#ifndef DB_CATALOG_H
#define DB_CATALOG_H

#define MAX_SP_ENTRIES 128

typedef struct {
    char key[64];      // Nombre lógico (ej: SP_USER_LOGIN)
    char real_sp[64];  // SP real en BD (ej: sp_auth_user_login)
} SP_Entry;

typedef struct {
    SP_Entry entries[MAX_SP_ENTRIES];
    int count;
} SP_Catalog;

int db_catalog_init(const char *filepath, SP_Catalog *catalog);
const char* db_catalog_get(const SP_Catalog *catalog, const char *key);

#endif
