#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>

#include "../system/routes.h"
#include "../system/hash.h"
#include "../system/db.h"            // Ahora utiliza únicamente la interfaz Blackbox
#include "../system/session_manager.h"
#include "users_service.h"

// Función auxiliar: obtiene valor de key=valor en el body (URL encoded)
static void get_value(const char *body, const char *key, char *out, size_t out_size) {
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
// BÚSQUEDA DE USUARIOS
// ===================================================================================================================== //

int get_user_service_name(const char *body, char *json_out, int out_size) {
    char name_str[100];
    char decoded_name[100];
    get_value(body, "user_name", name_str, sizeof(name_str));

    if (strlen(name_str) == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"Nombre no provisto\"}");
        return 0;
    }

    url_decode(decoded_name, name_str);

    const char *params[] = { decoded_name };
    DB_DataSet ds;

    if (db_call_sp("SP_GET_USER_BY_NAME", params, 1, &ds) != 0 || ds.num_rows == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"Usuario no encontrado\"}");
        db_free_dataset(&ds);
        return 0;
    }

    snprintf(json_out, out_size,
             "{\"status\": \"ok\", \"id\": %s, \"nombre\": \"%s\", \"email\": \"%s\", \"rol\": \"%s\"}",
             ds.data[0][0] ? ds.data[0][0] : "0",
             ds.data[0][1] ? ds.data[0][1] : "",
             ds.data[0][2] ? ds.data[0][2] : "",
             ds.data[0][3] ? ds.data[0][3] : "0");

    db_free_dataset(&ds);
    return 1;
}

int get_user_service_id(const char *body, char *json_out, int out_size) {
    char id_str[100];
    char decoded_id[100];
    get_value(body, "user_id", id_str, sizeof(id_str));

    if (strlen(id_str) == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"ID no provisto\"}");
        return 0;
    }

    url_decode(decoded_id, id_str);

    const char *params[] = { decoded_id };
    DB_DataSet ds;

    if (db_call_sp("SP_GET_USER_BY_ID", params, 1, &ds) != 0 || ds.num_rows == 0) {
        snprintf(json_out, out_size, "{\"status\": \"error\", \"message\": \"Usuario no encontrado\"}");
        db_free_dataset(&ds);
        return 0;
    }

    snprintf(json_out, out_size,
             "{\"status\": \"ok\", \"id\": %s, \"nombre\": \"%s\", \"email\": \"%s\", \"rol\": \"%s\"}",
             ds.data[0][0] ? ds.data[0][0] : "0",
             ds.data[0][1] ? ds.data[0][1] : "",
             ds.data[0][2] ? ds.data[0][2] : "",
             ds.data[0][3] ? ds.data[0][3] : "0");

    db_free_dataset(&ds);
    return 1;
}

static void route_get_user_by_name(int client, const char *body) {
    char response_json[1024];
    if (get_user_service_name(body, response_json, sizeof(response_json))) {
        send_response(client, "200 OK", "application/json", response_json);
    } else {
        send_response(client, "404 Not Found", "application/json", response_json);
    }
}

static void route_get_user_by_id(int client, const char *body) {
    char response_json[1024];
    if (get_user_service_id(body, response_json, sizeof(response_json))) {
        send_response(client, "200 OK", "application/json", response_json);
    } else {
        send_response(client, "404 Not Found", "application/json", response_json);
    }
}

// ===================================================================================================================== //
// REGISTRO DE USUARIOS
// ===================================================================================================================== //

int users_service_register(const char *body, char *error_msg, int error_size) {
    char nombre[128], email[128], password_1[128], password_2[128];
    char decoded_email[256], decoded_nombre[256];

    get_value(body, "nombre", nombre, sizeof(nombre));
    get_value(body, "email", email, sizeof(email));
    get_value(body, "password_1", password_1, sizeof(password_1));
    get_value(body, "password_2", password_2, sizeof(password_2));

    if (strlen(nombre) == 0 || strlen(email) == 0 || strlen(password_1) == 0 || strlen(password_2) == 0) {
        snprintf(error_msg, error_size, "missing required fields");
        return 0;
    }

    if (strcmp(password_1, password_2) != 0) {
        snprintf(error_msg, error_size, "Los Passwords no coinciden!");
        return 0;
    }

    url_decode(decoded_email, email);
    url_decode(decoded_nombre, nombre);

    // 1. Validar si ya existe el usuario/email
    const char *check_params[] = { decoded_nombre, decoded_email };
    DB_DataSet ds_check;

    if (db_call_sp("SP_CHECK_USER_EXISTS", check_params, 2, &ds_check) == 0 && ds_check.num_rows > 0) {
        int exists = atoi(ds_check.data[0][0]);
        db_free_dataset(&ds_check);

        if (exists > 0) {
            snprintf(error_msg, error_size, "email or user already exists");
            return 0;
        }
    } else {
        db_free_dataset(&ds_check);
    }

    // 2. Hash de contraseña
    char hash[SHA_DIGEST_LENGTH * 2 + 1];
    hash_password(password_1, "", hash);

    // 3. Insertar nuevo usuario mediante SP
    const char *reg_params[] = { decoded_nombre, decoded_email, hash, "0" };
    DB_DataSet ds_reg;

    if (db_call_sp("SP_REGISTER_USER", reg_params, 4, &ds_reg) != 0) {
        snprintf(error_msg, error_size, "database error: %s", ds_reg.error_msg);
        db_free_dataset(&ds_reg);
        return 0;
    }

    db_free_dataset(&ds_reg);
    return 1;
}

// ===================================================================================================================== //
// CAMBIO DE CONTRASEÑA
// ===================================================================================================================== //

int users_service_password(const char *body, char *error_msg, int error_size) {
    char email[128], password_1[128], password_2[128], decoded_email[256];

    get_value(body, "email", email, sizeof(email));
    get_value(body, "password_1", password_1, sizeof(password_1));
    get_value(body, "password_2", password_2, sizeof(password_2));

    if (strlen(email) == 0 || strlen(password_1) == 0 || strlen(password_2) == 0) {
        snprintf(error_msg, error_size, "Hay campos sin completar...");
        return 0;
    }

    if (strcmp(password_1, password_2) != 0) {
        snprintf(error_msg, error_size, "Los passwords no coinciden");
        return 0;
    }

    url_decode(decoded_email, email);

    // 1. Verificar si existe el email
    const char *check_params[] = { decoded_email };
    DB_DataSet ds_check;

    if (db_call_sp("SP_CHECK_EMAIL_EXISTS", check_params, 1, &ds_check) != 0 || ds_check.num_rows == 0) {
        snprintf(error_msg, error_size, "Usuario inexistente...");
        db_free_dataset(&ds_check);
        return 0;
    }

    int exists = atoi(ds_check.data[0][0]);
    db_free_dataset(&ds_check);

    if (exists <= 0) {
        snprintf(error_msg, error_size, "Usuario inexistente...");
        return 0;
    }

    // 2. Hash y Actualización de Contraseña
    char hash[SHA_DIGEST_LENGTH * 2 + 1];
    hash_password(password_1, "", hash);

    const char *update_params[] = { decoded_email, hash };
    DB_DataSet ds_update;

    if (db_call_sp("SP_UPDATE_USER_PASSWORD", update_params, 2, &ds_update) != 0) {
        snprintf(error_msg, error_size, "database error: %s", ds_update.error_msg);
        db_free_dataset(&ds_update);
        return 0;
    }

    db_free_dataset(&ds_update);
    return 1;
}

// ===================================================================================================================== //
// ACTUALIZACIÓN DE ROL
// ===================================================================================================================== //

int users_service_update_rol(const char* body, char* error_msg, int error_size) {
    char id_str[64], rol[32];

    get_value(body, "id", id_str, sizeof(id_str));
    get_value(body, "rol", rol, sizeof(rol));

    if (strlen(id_str) == 0 || strlen(rol) == 0) {
        snprintf(error_msg, error_size, "Hay campos sin datos...");
        return 0;
    }

    const char *params[] = { id_str, rol };
    DB_DataSet ds;

    if (db_call_sp("SP_UPDATE_USER_ROLE", params, 2, &ds) != 0) {
        snprintf(error_msg, error_size, "database error: %s", ds.error_msg);
        db_free_dataset(&ds);
        return 0;
    }

    db_free_dataset(&ds);
    return 1;
}

// ===================================================================================================================== //
// INICIO DE SESIÓN (LOGIN)
// ===================================================================================================================== //

int users_service_login(const char *email, const char *password, char *error_msg, UserData *user_out) {
    char decoded_email[256];
    url_decode(decoded_email, email);

    const char *params[] = { decoded_email };
    DB_DataSet ds;

    if (db_call_sp("SP_USER_LOGIN", params, 1, &ds) != 0 || ds.num_rows == 0) {
        strcpy(error_msg, "Usuario no encontrado");
        db_free_dataset(&ds);
        return 0;
    }

    const char *db_password = ds.data[0][3];
    if (!db_password || strlen(db_password) == 0) {
        strcpy(error_msg, "Contraseña no disponible");
        db_free_dataset(&ds);
        return 0;
    }

    char hashed_password[SHA_DIGEST_LENGTH * 2 + 1];
    hash_password(password, "", hashed_password);

    if (strcmp(db_password, hashed_password) != 0) {
        strcpy(error_msg, "Contraseña incorrecta");
        db_free_dataset(&ds);
        return 0;
    }

    // Mapeo genérico a la estructura UserData
    user_out->id = atoi(ds.data[0][0]);

    strncpy(user_out->nombre, ds.data[0][1], sizeof(user_out->nombre) - 1);
    user_out->nombre[sizeof(user_out->nombre) - 1] = '\0';

    strncpy(user_out->email, ds.data[0][2], sizeof(user_out->email) - 1);
    user_out->email[sizeof(user_out->email) - 1] = '\0';

    user_out->rol_id = atoi(ds.data[0][4]);

    db_free_dataset(&ds);
    return 1;
}

static void route_get_login(int client, const char *body) {
    (void)body;
    send_response(client, "200 OK", "application/json", "{ \"message\": \"login\" }");
}

static void route_post_login(int client, const char *body) {
    char email[256] = {0}, password[256] = {0};

    get_value(body, "email", email, sizeof(email));
    get_value(body, "password", password, sizeof(password));

    char error_msg[256];
    UserData user;
    user.session_token = tokenGenerator();

    if (users_service_login(email, password, error_msg, &user)) {
        UserSession* sesion_creada = session_manager_crear(user.nombre, user.rol_id, user.cod_inst, user.session_token);

        if (sesion_creada != NULL) {
            char response[1024];
            snprintf(response, sizeof(response),
                     "{ \"status\": \"success\", \"message\": \"Login correcto\", "
                     "\"user\": { \"id\": %d, \"nombre\": \"%s\", \"email\": \"%s\", \"rol_id\": %d, \"token\": \"%s\" } }",
                     user.id, user.nombre, user.email, user.rol_id, user.session_token);

            send_response(client, "200 OK", "application/json", response);
        } else {
            send_response(client, "500 Internal Error", "application/json", "{\"status\": \"error\", \"message\": \"No se pudo iniciar sesion en RAM\"}");
        }
    } else {
        char response[512];
        snprintf(response, sizeof(response), "{\"status\": \"error\", \"message\": \"%s\"}", error_msg);
        send_response(client, "401 Unauthorized", "application/json", response);
    }

    if (user.session_token) {
        free(user.session_token);
    }
}

// ===================================================================================================================== //
// LISTAR Y CONSULTAR ROLES
// ===================================================================================================================== //

static void users_service_list(int client, const char *body) {
    (void)body;

    DB_DataSet ds;

    if (db_call_sp("SP_GET_USERS_LIST", NULL, 0, &ds) != 0) {
        send_response(client, "500 Internal Server Error", "application/json",
                      "{\"status\": \"error\", \"message\": \"Error al consultar usuarios\"}");
        return;
    }

    char buffer[8192];
    size_t offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[");

    for (int r = 0; r < ds.num_rows; r++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "%s{\"id\": %s, \"nombre\": \"%s\", \"email\": \"%s\", \"rol\": \"%s\"}",
                           (r > 0) ? ", " : "",
                           ds.data[r][0] ? ds.data[r][0] : "null",
                           ds.data[r][1] ? ds.data[r][1] : "",
                           ds.data[r][2] ? ds.data[r][2] : "",
                           ds.data[r][3] ? ds.data[r][3] : "");
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "]");

    db_free_dataset(&ds);
    send_response(client, "200 OK", "application/json", buffer);
}

int users_service_get_rol(int client, const char *body) {
    char id_str[32];
    get_value(body, "id", id_str, sizeof(id_str));

    if (strlen(id_str) == 0) {
        send_response(client, "400 Bad Request", "application/json",
                      "{\"status\": \"error\", \"message\": \"USER ID no recibido\"}");
        return 0;
    }

    const char *params[] = { id_str };
    DB_DataSet ds;

    if (db_call_sp("SP_GET_USER_ROLE", params, 1, &ds) != 0 || ds.num_rows == 0) {
        send_response(client, "404 Not Found", "application/json",
                      "{\"status\": \"error\", \"message\": \"Usuario no encontrado\"}");
        db_free_dataset(&ds);
        return 0;
    }

    char response[256];
    snprintf(response, sizeof(response),
             "{\"status\": \"ok\", \"rol\": \"%s\"}", ds.data[0][0] ? ds.data[0][0] : "0");

    db_free_dataset(&ds);
    send_response(client, "200 OK", "application/json", response);

    return 1;
}

static void route_get_users_list(int client, const char *body) {
    users_service_list(client, body);
}

static void route_get_users_rol(int client, const char *body) {
    users_service_get_rol(client, body);
}

static void route_users_update_rol(int client, const char* body) {
    char error[256];
    if (users_service_update_rol(body, error, sizeof(error))) {
        send_response(client, "200 OK", "application/json",
                      "{\"status\": \"ok\", \"message\": \"Rol Actualizado Satisfactoriamente\"}");
    } else {
        char buf[512];
        snprintf(buf, sizeof(buf), "{\"status\": \"error\", \"message\": \"%s\"}", error);
        send_response(client, "400 Bad Request", "application/json", buf);
    }
}

// ===================================================================================================================== //
// GENERADOR DE TOKENS SEGURO Y REGISTRO DE RUTAS
// ===================================================================================================================== //

char* tokenGenerator(void) {
    static int rand_inicializado = 0;
    if (!rand_inicializado) {
        srand((unsigned int)time(NULL));
        rand_inicializado = 1;
    }

    char* token = malloc(TOKEN_LENGTH + 1);
    if (token == NULL) {
        return NULL;
    }

    const char caracteres[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int longitud_caracteres = sizeof(caracteres) - 1;

    for (int i = 0; i < TOKEN_LENGTH; i++) {
        int indice_aleatorio = rand() % longitud_caracteres;
        token[i] = caracteres[indice_aleatorio];
    }

    token[TOKEN_LENGTH] = '\0';
    return token;
}

void init_login_routes(void) {
    add_route("GET", "/login", route_get_login);
    add_route("POST", "/users/get-user", route_get_user_by_name);
    add_route("POST", "/login", route_post_login);
    add_route("GET", "/users/list", route_get_users_list);
    add_route("POST", "/users/get-role", route_get_users_rol);
    add_route("POST", "/users/update-role", route_users_update_rol);
    add_route("POST", "/users/get-id", route_get_user_by_id);
}
