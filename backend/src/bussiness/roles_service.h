#ifndef ROLES_SERVICE_H
#define ROLES_SERVICE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../system/ArrayList.h"

typedef struct {
    int id;
    char descripcion[100];
} Roles;

// Constructor
Roles* new_role(void);

// Módulos de Lógica y Memoria
void roles_init_cache(ArrayList* alist_roles);
void roles_load_storage(ArrayList* alist_roles);
int roles_service_register(const char *body, char *error_msg, int error_size);
int roles_service_edit(const char *body, char *error_msg, int error_size);
int get_roles_service_id(const char *body, char *json_out, int out_size);

// Inicializador de Rutas HTTP
void init_roles_routes(void);

#endif
