#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env_loader.h"

int env_load_config(const char *filepath, DB_Config *config) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "[ENV] Error al abrir %s\n", filepath);
        return -1;
    }

    // Valores por defecto
    memset(config, 0, sizeof(DB_Config));
    config->port = 3306;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char key[64], val[128];
        if (sscanf(line, "%63[^=]=%127[^\r\n]", key, val) == 2) {
            if (strcmp(key, "DB_HOST") == 0) strncpy(config->host, val, sizeof(config->host) - 1);
            else if (strcmp(key, "DB_PORT") == 0) config->port = atoi(val);
            else if (strcmp(key, "DB_USER") == 0) strncpy(config->user, val, sizeof(config->user) - 1);
            else if (strcmp(key, "DB_PASS") == 0) strncpy(config->pass, val, sizeof(config->pass) - 1);
            else if (strcmp(key, "DB_NAME") == 0) strncpy(config->dbname, val, sizeof(config->dbname) - 1);
            else if (strcmp(key, "DB_PREFIX") == 0) strncpy(config->prefix, val, sizeof(config->prefix) - 1);
        }
    }

    fclose(file);
    return 0;
}
