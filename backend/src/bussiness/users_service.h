#ifndef USERS_SERVICE_H
#define USERS_SERVICE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_LENGTH 128

typedef struct {
    int  id;
    char nombre[128];
    char email[128];
    char cod_inst[5];
    int  rol_id;
    char* session_token;
} UserData;

// --- Funciones del Servicio de Usuarios ---
int users_service_register(const char *body, char *error_msg, int error_size);
int users_service_login(const char *email, const char *password, char *error_msg, UserData *user_out);
int users_service_password(const char *body, char *error_msg, int error_size);
int users_service_get_rol(int client, const char *body);
int users_service_update_rol(const char* body, char* error_msg, int error_size);
int get_user_service_name(const char *body, char *json_out, int out_size);
int get_user_service_id(const char *body, char *json_out, int out_size);

// --- Utilidades y Rutas ---
char* tokenGenerator(void);
void init_login_routes(void);

#endif
