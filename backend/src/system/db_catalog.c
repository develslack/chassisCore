#include <stdio.h>
#include <string.h>
#include "db_catalog.h"

int db_catalog_init(const char *filepath, SP_Catalog *catalog) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "[CATALOG] Error al abrir %s\n", filepath);
        return -1;
    }

    catalog->count = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) && catalog->count < MAX_SP_ENTRIES) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char key[64], sp[64];
        if (sscanf(line, "%63[^=]=%63[^\r\n]", key, sp) == 2) {
            strncpy(catalog->entries[catalog->count].key, key, 63);
            strncpy(catalog->entries[catalog->count].real_sp, sp, 63);
            catalog->count++;
        }
    }

    fclose(file);
    printf("[CATALOG] %d Stored Procedures cargados correctamente.\n", catalog->count);
    return 0;
}

const char* db_catalog_get(const SP_Catalog *catalog, const char *key) {
    for (int i = 0; i < catalog->count; i++) {
        if (strcmp(catalog->entries[i].key, key) == 0) {
            return catalog->entries[i].real_sp;
        }
    }
    return NULL;
}
