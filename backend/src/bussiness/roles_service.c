#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../system/hash.h"
#include "../system/routes.h"
#include "../system/db.h"
#include "../system/ArrayList.h"
#include "roles_service.h"

// Variable local para acceder a la memoria (ArrayList)
static ArrayList* pListRolesLocal = NULL;

// ===================================================================================================================== //
// CONSTRUCTOR Y AUXILIARES
// ===================================================================================================================== //

Roles* new_role(void) {
    Roles* oneRole = (Roles*)malloc(sizeof(Roles));
    if (oneRole != NULL) {
        memset(oneRole, 0, sizeof(Roles));
    }
    return oneRole;
}

static void get_roles_value(const char *body, const char *key, char *out, size_t out_size) {
    char *pos = strstr(body, key);
    if (!pos) {
        out[0] = '\0';
        return;
    }
    pos += strlen(key);
    if (*pos == '=') pos++;
    const char *end = strchr(pos, '&');
    size_t len = end ? (size_t)(end - pos) : strlen(pos);
    if (len >= out_size) len = out_size - 1;
    strncpy(out, pos, len);
    out[len] = '\0';
}

// ===================================================================================================================== //
// INICIALIZACIÓN Y CARGA EN MEMORIA (CACHE)
// ===================================================================================================================== //

void roles_init_cache(ArrayList* alist_roles) {
    if (alist_roles != NULL) {
        pListRolesLocal = alist_roles;
        printf("===================================================================================\n");
        printf("✅ Negociando espacio en memoria para el servicio de Roles.\n");
    } else {
        printf("===================================================================================\n");
        printf("⚠️ Advertencia: Se intentó inicializar la caché de Roles con NULL.\n");
    }
}

void roles_load_storage(ArrayList* alist_roles) {
    if (alist_roles == NULL) return;

    DB_DataSet ds;
    if (db_call_sp("SP_GET_ALL_ROLES", NULL, 0, &ds) != 0) {
        fprintf(stderr, "❌ Error al cargar almacenamiento de roles.\n");
        db_free_dataset(&ds);
        return;
    }

    for (int i = 0; i < ds.num_rows; i++) {
        Roles* oneRole = new_role();
        if (oneRole) {
            oneRole->id = atoi(ds.data[i][0] ? ds.data[i][0] : "0");
            strncpy(oneRole->descripcion, ds.data[i][1] ? ds.data[i][1] : "", sizeof(oneRole->descripcion) - 1);
            alist_roles->add(alist_roles, oneRole);
        }
    }

    db_free_dataset(&ds);
    printf("===================================================================================\n");
    printf("📊 Memoria: %d roles cargados. Espacio reservado: %d slots.\n", alist_roles->len(alist_roles), alist_roles->reservedSize);
}

// ===================================================================================================================== //
// REGISTRO DE ROL
// ===================================================================================================================== //

int roles_service_register(const char *body, char *error_msg, int error_size) {
    char descripcion[128];
    char d_descripcion[128];

    get_roles_value(body, "descripcion", descripcion, sizeof(descripcion));
    url_decode(d_descripcion, descripcion);

    if (strlen(d_descripcion) == 0) {
        snprintf(error_msg, error_size, "La descripción es obligatoria.");
        return 0;
    }

    // 1. Verificación de duplicados en RAM
    if (pListRolesLocal != NULL) {
        for (int i = 0; i < pListRolesLocal->len(pListRolesLocal); i++) {
            Roles* r = (Roles*) pListRolesLocal->get(pListRolesLocal, i);
            if (r && strcasecmp(r->descripcion, d_descripcion) == 0) {
                snprintf(error_msg, error_size, "Error: El rol '%s' ya existe.", d_descripcion);
                return 0;
            }
        }
    }

    // 2. Inserción en BD mediante SP
    const char *params[] = { d_descripcion };
    DB_DataSet ds;

    if (db_call_sp("SP_INSERT_ROLE", params, 1, &ds) != 0) {
        snprintf(error_msg, error_size, "Error interno al guardar en la base de datos.");
        db_free_dataset(&ds);
        return 0;
    }

    int nuevo_id = (ds.num_rows > 0 && ds.data[0][0]) ? atoi(ds.data[0][0]) : (int)ds.last_insert_id;
    db_free_dataset(&ds);

    if (nuevo_id <= 0) {
        snprintf(error_msg, error_size, "Error interno al obtener el ID del rol insertado.");
        return 0;
    }

    // 3. Sincronización inmediata con el ArrayList en RAM
    Roles* nuevoRol = new_role();
    if (nuevoRol && pListRolesLocal != NULL) {
        nuevoRol->id = nuevo_id;
        strncpy(nuevoRol->descripcion, d_descripcion, sizeof(nuevoRol->descripcion) - 1);
        pListRolesLocal->add(pListRolesLocal, nuevoRol);

        printf("✅ Sincronización exitosa: Rol '%s' (ID: %d) añadido a RAM.\n", d_descripcion, nuevo_id);
    }

    return 1;
}

// ===================================================================================================================== //
// EDICIÓN DE ROL
// ===================================================================================================================== //

int roles_service_edit(const char *body, char *error_msg, int error_size) {
    char id_str[32];
    char descripcion[100];
    char d_descripcion[100];

    get_roles_value(body, "id", id_str, sizeof(id_str));
    get_roles_value(body, "descripcion", descripcion, sizeof(descripcion));
    url_decode(d_descripcion, descripcion);

    int id_a_editar = atoi(id_str);

    if (id_a_editar <= 0 || strlen(d_descripcion) == 0) {
        snprintf(error_msg, error_size, "ID o descripción no completados.");
        return 0;
    }

    // 1. Verificación de existencia y duplicados en RAM
    if (pListRolesLocal != NULL) {
        for (int i = 0; i < pListRolesLocal->len(pListRolesLocal); i++) {
            Roles* r = (Roles*) pListRolesLocal->get(pListRolesLocal, i);
            if (r && r->id != id_a_editar && strcasecmp(r->descripcion, d_descripcion) == 0) {
                snprintf(error_msg, error_size, "Error: Rol Existente.");
                return 0;
            }
        }
    }

    // 2. Actualización en BD mediante SP
    const char *params[] = { id_str, d_descripcion };
    DB_DataSet ds;

    if (db_call_sp("SP_EDIT_ROLE", params, 2, &ds) != 0) {
        snprintf(error_msg, error_size, "Error al actualizar en la base de datos.");
        db_free_dataset(&ds);
        return 0;
    }

    db_free_dataset(&ds);

    // 3. Actualización en RAM (ArrayList)
    if (pListRolesLocal != NULL) {
        for (int i = 0; i < pListRolesLocal->len(pListRolesLocal); i++) {
            Roles* r = (Roles*) pListRolesLocal->get(pListRolesLocal, i);
            if (r && r->id == id_a_editar) {
                strncpy(r->descripcion, d_descripcion, sizeof(r->descripcion) - 1);
                printf("✅ Memoria sincronizada: Rol ID %d actualizado a '%s'.\n", id_a_editar, d_descripcion);
                break;
            }
        }
    }

    return 1;
}

// ===================================================================================================================== //
// CONSULTA POR ID
// ===================================================================================================================== //

int get_roles_service_id(const char *body, char *json_out, int out_size) {
    char id_str[10];
    get_roles_value(body, "id", id_str, sizeof(id_str));

    if (strlen(id_str) == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"ID no provisto\"}");
        return 0;
    }

    const char *params[] = { id_str };
    DB_DataSet ds;

    if (db_call_sp("SP_GET_ROLE_BY_ID", params, 1, &ds) != 0 || ds.num_rows == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"Rol no encontrado\"}");
        db_free_dataset(&ds);
        return 0;
    }

    snprintf(json_out, out_size,
             "{\"status\": \"ok\", \"id\": %s, \"descripcion\": \"%s\"}",
             ds.data[0][0] ? ds.data[0][0] : "0",
             ds.data[0][1] ? ds.data[0][1] : "");

    db_free_dataset(&ds);
    return 1;
}

// ===================================================================================================================== //
// HANDLERS Y RUTAS HTTP
// ===================================================================================================================== //

static void route_post_roles(int client, const char *body) {
    char error_msg[256];
    if (roles_service_register(body, error_msg, sizeof(error_msg))) {
        send_response(client, "200 OK", "application/json", "{\"status\": \"ok\", \"message\": \"Rol creado y caché actualizada\"}");
    } else {
        char response[512];
        snprintf(response, sizeof(response), "{\"status\": \"error\", \"message\": \"%s\"}", error_msg);
        send_response(client, "400 Bad Request", "application/json", response);
    }
}

static void route_post_roles_edit(int client, const char *body) {
    char error_msg[256];
    if (roles_service_edit(body, error_msg, sizeof(error_msg))) {
        send_response(client, "200 OK", "application/json", "{\"status\": \"ok\", \"message\": \"Rol actualizado correctamente\"}");
    } else {
        char response[512];
        snprintf(response, sizeof(response), "{\"status\": \"error\", \"message\": \"%s\"}", error_msg);
        send_response(client, "400 Bad Request", "application/json", response);
    }
}

static void route_get_roles_list(int client, const char *body) {
    (void)body;

    if (pListRolesLocal == NULL) {
        printf("❌ Error crítico: pListRolesLocal es NULL en el handler.\n");
        send_response(client, "500 Internal Error", "application/json", "{\"error\":\"Error de vinculación de memoria\"}");
        return;
    }

    size_t total_roles = pListRolesLocal->len(pListRolesLocal);
    size_t buffer_size = (total_roles * 150) + 10;
    char *json = (char*) calloc(1, buffer_size);

    if (json == NULL) {
        send_response(client, "500 Internal Server Error", "text/plain", "Error de memoria");
        return;
    }

    strcpy(json, "[");
    for (size_t i = 0; i < total_roles; i++) {
        Roles* oneRole = (Roles*) pListRolesLocal->get(pListRolesLocal, i);
        if (oneRole) {
            char item[200];
            snprintf(item, sizeof(item),
                     "{\"id\": %d, \"descripcion\": \"%s\"}%s",
                     oneRole->id, oneRole->descripcion, (i < total_roles - 1) ? ", " : "");
            strcat(json, item);
        }
    }
    strcat(json, "]");

    send_response(client, "200 OK", "application/json", json);
    free(json);
}

static void route_get_roles_by_id(int client, const char *body) {
    char response_json[1024];
    if (get_roles_service_id(body, response_json, sizeof(response_json))) {
        send_response(client, "200 OK", "application/json", response_json);
    } else {
        send_response(client, "404 Not Found", "application/json", response_json);
    }
}

void init_roles_routes(void) {
    add_route("GET", "/roles/list", route_get_roles_list);
    add_route("POST", "/roles/add", route_post_roles);
    add_route("POST", "/roles/edit", route_post_roles_edit);
    add_route("POST", "/roles/get", route_get_roles_by_id);
}
