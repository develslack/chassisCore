#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "system/system_struct.h"
#include "system/frontend_server.h"
#include "system/backend_server.h"
#include "system/session_manager.h"
#include "system/ArrayList.h"
#include "system/db.h"               // 👈 Fachada Blackbox de Base de Datos

#include "bussiness/roles_service.h"
#include "bussiness/users_service.h"

int main() {

    // 1. Limpieza visual de la terminal de Slackware/Debian
    system("clear");
    printf("=== AUDITORÍA DE INFRAESTRUCTURA DE SISTEMA (FAIL-FAST) ===\n");

    // 2. Activamos el blindaje de sockets primero
    system_blindar_senales();

    // 3. Freno de mano: Si hay menos de 4 núcleos, esta función imprime el error,
    // escribe en el log y ejecuta un exit(EXIT_FAILURE), frenando el main ACÁ mismo.
    system_verificar_y_mapear_hardware();

    // 4. Iniciamos session_manager como controlador del sistema
    session_manager_init();


    // ================================================================================= //
    // 🚀 SI LLEGA A ESTE PUNTO, EL HARDWARE ESTÁ BLINDADO Y ES APTO PARA 24x365
    // ================================================================================= //
    printf("\n=== INICIANDO MOTOR BLACKBOX Y CONFIGURACIÓN DE MEMORIA ===\n");

    // 5. Inicializar Motor de BD (Lee config/.env, escanea prefijos/tablas y carga queries/sp_catalog.conf)
    if (db_init("../config/.env", "../queries/sp_catalog.conf") != 0) {
        fprintf(stderr, "❌ [FATAL] Error crítico al conectar e inicializar el motor de Base de Datos. Abortando.\n");
        exit(EXIT_FAILURE);
    }

    // 6. Registrar las rutas de los módulos del negocio
    init_login_routes();

    printf("\n=== LEVANTANDO SERVIDORES WEB Y API ===\n");

    // 🔹 Levantar frontend
    start_frontend_server(5001, "../../frontend/templates/views");

    // 🔹 Luego iniciar backend normalmente
    start_backend(5000);

    // 🔹 Cierre del motor al finalizar la ejecución
    db_close();

    return 0;
}
