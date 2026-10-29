#ifndef ENV_LOADER_H
#define ENV_LOADER_H

typedef struct {
    char host[128];
    int port;
    char user[64];
    char pass[64];
    char dbname[64];
    char prefix[32];
} DB_Config;

int env_load_config(const char *filepath, DB_Config *config);

#endif
