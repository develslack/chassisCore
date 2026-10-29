#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "routes.h"
#include "env_loader.h"
#include "theme_handler.h"

static void get_query_param(const char *body, const char *key, char *out, size_t out_size) {
    char *pos = strstr(body, key);
    if (!pos) { out[0] = '\0'; return; }
    pos += strlen(key);
    if (*pos == '=') pos++;
    const char *end = strchr(pos, '&');
    size_t len = end ? (size_t)(end - pos) : strlen(pos);
    if (len >= out_size) len = out_size - 1;
    strncpy(out, pos, len);
    out[len] = '\0';
}

// GET /theme/get?user=username
static void route_get_theme(int client, const char *body) {
    char username[64] = {0};
    get_query_param(body, "user", username, sizeof(username));

    if (strlen(username) == 0) {
        strncpy(username, "default", sizeof(username) - 1);
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "frontend/theme_config/%s-theme.conf", username);

    char theme[64] = DEFAULT_THEME;
    FILE *f = fopen(filepath, "r");
    if (f) {
        char line[128];
        if (fgets(line, sizeof(line), f)) {
            sscanf(line, "theme=%63[^\r\n]", theme);
        }
        fclose(f);
    }

    char json[256];
    snprintf(json, sizeof(json), "{\"status\": \"ok\", \"theme\": \"%s\"}", theme);
    send_response(client, "200 OK", "application/json", json);
}

// POST /theme/set (body: user=username&theme=theme-name)
static void route_post_theme(int client, const char *body) {
    char username[64] = {0};
    char theme[64] = {0};

    get_query_param(body, "user", username, sizeof(username));
    get_query_param(body, "theme", theme, sizeof(theme));

    if (strlen(username) == 0 || strlen(theme) == 0) {
        send_response(client, "400 Bad Request", "application/json",
                      "{\"status\": \"error\", \"message\": \"Faltan datos de usuario o tema\"}");
        return;
    }

    // Si el archivo .env no existe, estamos operando en Modo DEMO pura
    int is_demo = (access("backend/config/.env", F_OK) != 0);

    if (is_demo) {
        // En modo DEMO confirmamos la respuesta pero OMITIMOS escribir en disco
        send_response(client, "200 OK", "application/json",
                      "{\"status\": \"ok\", \"message\": \"Tema aplicado temporalmente (Modo DEMO)\"}");
        return;
    }

    // Guardar en disco para el usuario actual
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "frontend/theme_config/%s-theme.conf", username);

    FILE *f = fopen(filepath, "w");
    if (!f) {
        send_response(client, "500 Internal Error", "application/json",
                      "{\"status\": \"error\", \"message\": \"No se pudo guardar la configuración\"}");
        return;
    }

    fprintf(f, "theme=%s\n", theme);
    fclose(f);

    send_response(client, "200 OK", "application/json",
                  "{\"status\": \"ok\", \"message\": \"Tema guardado correctamente\"}");
}

void init_theme_routes(void) {
    add_route("GET", "/theme/get", route_get_theme);
    add_route("POST", "/theme/set", route_post_theme);
}
