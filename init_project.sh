#!/bin/bash
# =================================================================
# chassisCore - Interactive Project Bootstrapper & Explorer
# =================================================================

clear
echo "======================================================="
echo "   chassisCore - BIENVENIDO AL FRAMEWORK C / SPA       "
echo "======================================================="

# Garantizar que trabajamos desde la raíz del proyecto
CDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$CDIR" || exit 1

# -----------------------------------------------------------------
# PASO 1: AUDITORÍA DE REQUISITOS DE SISTEMA Y LIBRERÍAS C
# -----------------------------------------------------------------
echo -e "\n[1/4] Auditando Requisitos de Sistema y Librerías C..."

if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_NAME=$NAME
else
    OS_NAME=$(uname -s)
fi
echo "-> Sistema Operativo detectado: $OS_NAME"

MISSING_DEPS=0

check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e " \033[0;31m[FAILS]\033[0m Herramienta no encontrada: $1"
        MISSING_DEPS=$((MISSING_DEPS + 1))
    else
        echo -e " \033[0;32m[ OK ]\033[0m Herramienta instalada: $1"
    fi
}

check_header() {
    local header=$1
    if [ ! -f "/usr/include/$header" ] && [ ! -f "/usr/local/include/$header" ] && [ ! -f "/usr/include/mariadb/$header" ] && [ ! -f "/usr/include/mysql/$header" ]; then
        echo -e " \033[0;31m[FAILS]\033[0m Librería C / Header faltante: $header"
        MISSING_DEPS=$((MISSING_DEPS + 1))
    else
        echo -e " \033[0;32m[ OK ]\033[0m Librería C instalada: $header"
    fi
}

check_command "gcc"
check_header "openssl/sha.h"

# Auditoría e Instalación de conio.h
if [ ! -f "/usr/include/conio.h" ]; then
    echo -e " \033[0;33m[AVISO]\033[0m No se encontró 'conio.h' en el sistema."

    if [ -d "conio_lib" ] && [ -f "conio_lib/conio.h" ]; then
        echo -e "-> Copiando librería conio desde 'conio_lib/' (Se requerirá contraseña de sudo)..."

        sudo cp conio_lib/conio.h /usr/include/

        if [ -d "/usr/lib64" ]; then
            sudo cp conio_lib/libconia.a /usr/lib64/ 2>/dev/null || sudo cp conio_lib/* /usr/lib64/ 2>/dev/null
        fi
        if [ -d "/usr/lib" ]; then
            sudo cp conio_lib/libconia.a /usr/lib/ 2>/dev/null || sudo cp conio_lib/* /usr/lib/ 2>/dev/null
        fi

        if [ -f "/usr/include/conio.h" ]; then
            echo -e " \033[0;32m[ OK ]\033[0m Librería conio instalada correctamente en el sistema."
        else
            echo -e " \033[0;31m[FAILS]\033[0m Error al copiar la librería conio al sistema."
            MISSING_DEPS=$((MISSING_DEPS + 1))
        fi
    else
        echo -e " \033[0;31m[FAILS]\033[0m No se encontró el directorio local 'conio_lib/' con los archivos de origen."
        MISSING_DEPS=$((MISSING_DEPS + 1))
    fi
else
    echo -e " \033[0;32m[ OK ]\033[0m Librería C instalada: conio.h"
fi

if [ $MISSING_DEPS -gt 0 ]; then
    echo -e "\n\033[0;31m❌ Error Crítico: Faltan $MISSING_DEPS dependencias requeridas en su sistema. Instálelas y reintente.\033[0m"
    exit 1
fi

echo -e "\033[0;32m✅ Auditoría de requisitos completada exitosamente.\033[0m"

# -----------------------------------------------------------------
# PASO 2: MENÚ INTERACTIVO DE SELECCIÓN
# -----------------------------------------------------------------
echo -e "\n======================================================="
echo "            SELECCIONE EL MODO DE EJECUCIÓN           "
echo "======================================================="
echo "  [1] Probar la App DEMO ('chassisCore' precompilado)"
echo "  [2] Crear una NUEVA Aplicación (Scaffolding con BD)"
echo "  [3] Salir"
echo "======================================================="
read -p "Seleccione una opción [1-3]: " MODO_OPCION

case $MODO_OPCION in
    1)
        MODE="DEMO"
        APP_NAME="chassisCore"
        BIN_NAME="chassisCore"
        echo -e "\n-> \033[0;36mIniciando en Modo DEMO...\033[0m"
        ;;
    2)
        MODE="NEW_APP"
        ;;
    3)
        echo "Saliendo del asistente."
        exit 0
        ;;
    *)
        echo -e "\033[0;31mOpción inválida. Abortando.\033[0m"
        exit 1
        ;;
esac

# -----------------------------------------------------------------
# PASO 3: LÓGICA SEGÚN MODO SELECCIONADO
# -----------------------------------------------------------------

# --- OPCIÓN 1: MODO DEMO ---
if [ "$MODE" == "DEMO" ]; then
    echo -e "\n[2/3] Verificando ejecutable DEMO (backend/bin/chassisCore)..."

    if [ ! -f "backend/bin/chassisCore" ]; then
        echo -e "\033[0;33m⚠️ No se encontró 'backend/bin/chassisCore'. Compilando por única vez...\033[0m"
        mkdir -p backend/bin
        gcc -Wall -g -std=gnu11 -o "backend/bin/chassisCore" \
            backend/src/system/ArrayList.c \
            backend/src/system/commonlib.c \
            backend/src/system/db.c \
            backend/src/system/auth.c \
            backend/src/system/users.c \
            backend/src/system/routes.c \
            backend/src/system/hash.c \
            backend/src/system/frontend_server.c \
            backend/src/system/backend_server.c \
            backend/src/system/session_manager.c \
            backend/src/system/system_struct.c \
            backend/src/system/open_docs.c \
            backend/src/system/db_catalog.c \
            backend/src/system/db_schemas.c \
            backend/src/system/env_loader.c \
            backend/src/system/theme_handler.c \
            backend/src/bussiness/roles_service.c \
            backend/src/bussiness/users_service.c \
            backend/src/main.c \
            -lm -lpthread -lmysqlclient -lssl -lcrypto -lconia 2>&1 | tee comp_err.txt
    else
        echo -e "\033[0;32m✅ Binario DEMO listo en backend/bin/chassisCore.\033[0m"
    fi

    echo -e "\n[3/3] Iniciando 'chassisCore' en modo desarrollo..."

    APP_URL="http://localhost:5001/dashboard"

    open_browser() {
        local url=$1
        sleep 1.5
        if command -v xdg-open &> /dev/null; then
            xdg-open "$url" &> /dev/null &
        elif command -v sensible-browser &> /dev/null; then
            sensible-browser "$url" &> /dev/null &
        elif command -v x-www-browser &> /dev/null; then
            x-www-browser "$url" &> /dev/null &
        elif command -v firefox &> /dev/null; then
            firefox "$url" &> /dev/null &
        elif command -v google-chrome &> /dev/null; then
            google-chrome "$url" &> /dev/null &
        fi
    }

    open_browser "$APP_URL"
    cd backend/bin/
    ./chassisCore
    exit 0
fi

# --- OPCIÓN 2: MODO NUEVA APLICACIÓN ---
if [ "$MODE" == "NEW_APP" ]; then
    echo -e "\n--- INSTANCIACIÓN DE NUEVA APLICACIÓN ---"
    read -p "Ingrese el nombre de su nueva aplicación (ej: Music Center): " APP_NAME
    while [ -z "$APP_NAME" ]; do
        echo -e "\033[0;31mEl nombre no puede estar vacío.\033[0m"
        read -p "Ingrese el nombre de su nueva aplicación: " APP_NAME
    done

    APP_DIR_NAME=$(echo "$APP_NAME" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/_/g')
    BIN_NAME="$APP_DIR_NAME"
    CURRENT_DIR_NAME=$(basename "$PWD")

    if [ "$CURRENT_DIR_NAME" != "$APP_DIR_NAME" ]; then
        TARGET_PATH="../$APP_DIR_NAME"

        if [ -d "$TARGET_PATH" ]; then
            echo -e "\033[0;33m⚠️ El directorio '$TARGET_PATH' ya existe.\033[0m"
            read -p "¿Desea ingresar a él y continuar? [S/n]: " RESP_DIR
            if [[ "$RESP_DIR" == "n" || "$RESP_DIR" == "N" ]]; then
                echo "Operación cancelada."
                exit 0
            fi
        else
            echo -e "-> Generando espejo del framework en: \033[0;32m$TARGET_PATH\033[0m ..."
            cp -r . "$TARGET_PATH"
            rm -rf "$TARGET_PATH/.git" 2>/dev/null
        fi

        echo -e "-> Trasladando ejecución a la carpeta del nuevo proyecto: \033[0;32m$TARGET_PATH\033[0m"
        echo "======================================================="

        cd "$TARGET_PATH" || exit 1
    fi

    # Compilar ejecutable personalizado
    mkdir -p backend/bin backend/config backend/queries backend/sql
    echo -e "\n[2/4] Compilando ejecutable personalizado (backend/bin/$BIN_NAME)..."

    gcc -Wall -g -std=gnu11 -o "backend/bin/$BIN_NAME" \
        backend/src/system/ArrayList.c \
        backend/src/system/commonlib.c \
        backend/src/system/db.c \
        backend/src/system/auth.c \
        backend/src/system/users.c \
        backend/src/system/routes.c \
        backend/src/system/hash.c \
        backend/src/system/frontend_server.c \
        backend/src/system/backend_server.c \
        backend/src/system/session_manager.c \
        backend/src/system/system_struct.c \
        backend/src/system/open_docs.c \
        backend/src/system/db_catalog.c \
        backend/src/system/db_schemas.c \
        backend/src/system/env_loader.c \
        backend/src/system/theme_handler.c \
        backend/src/bussiness/roles_service.c \
        backend/src/bussiness/users_service.c \
        backend/src/main.c \
        -lm -lpthread -lmysqlclient -lssl -lcrypto -lconia 2>&1 | tee comp_err.txt

    if [ ! -f "backend/bin/$BIN_NAME" ]; then
        echo -e "\n\033[0;31m❌ Error en la compilación. Revisa comp_err.txt\033[0m"
        exit 1
    fi

    echo -e "\033[0;32m✅ COMPILACIÓN EXITOSA: backend/bin/$BIN_NAME\033[0m"

    # Configurar BD
    echo -e "\n[3/4] Verificando Servidor MariaDB/MySQL..."

    if ! mysqladmin ping --silent &>/dev/null && ! systemctl is-active --quiet mariadb && ! systemctl is-active --quiet mysql; then
        echo -e "\033[0;33m⚠️ Advertencia: No se detectó un servidor MySQL/MariaDB activo localmente.\033[0m"
        read -p "¿Desea intentar continuar con la configuración de BD? [s/N]: " RESP_DB
        if [[ "$RESP_DB" != "s" && "$RESP_DB" != "S" ]]; then
            echo "Saliendo sin inicializar base de datos. Binario guardado en backend/bin/$BIN_NAME"
            exit 0
        fi
    fi

    echo -e "\n--- CONFIGURACIÓN INICIAL DE BASE DE DATOS ---"

    CLEAN_NAME=$(echo "$APP_NAME" | sed 's/[^a-zA-Z0-9 ]//g')
    PREFIX=""
    for word in $CLEAN_NAME; do
        PREFIX="${PREFIX}${word:0:1}"
    done
    PREFIX=$(echo "${PREFIX}_" | tr '[:upper:]' '[:lower:]')

    read -p "Host de BD [127.0.0.1]: " DB_HOST
    DB_HOST=${DB_HOST:-127.0.0.1}

    read -p "Puerto de BD [3306]: " DB_PORT
    DB_PORT=${DB_PORT:-3306}

    read -p "Usuario DB [root]: " DB_USER
    DB_USER=${DB_USER:-root}

    read -s -p "Contraseña DB: " DB_PASS
    echo ""

    read -p "Nombre de la Base de Datos a crear: " DB_NAME
    while [ -z "$DB_NAME" ]; do
        echo -e "\033[0;31mEl nombre de la base de datos es obligatorio.\033[0m"
        read -p "Nombre de la Base de Datos: " DB_NAME
    done

    echo -e "\n--- CREACIÓN DEL USUARIO ADMINISTRADOR ROOT ---"
    read -p "Nombre del Usuario Root [admin_root]: " ROOT_NAME
    ROOT_NAME=${ROOT_NAME:-admin_root}

    read -p "Email del Usuario Root: " ROOT_EMAIL
    while [ -z "$ROOT_EMAIL" ]; do
        echo -e "\033[0;31mEl email es obligatorio.\033[0m"
        read -p "Email del Usuario Root: " ROOT_EMAIL
    done

    read -s -p "Contraseña para Usuario Root: " ROOT_PASS
    echo ""
    while [ -z "$ROOT_PASS" ]; do
        echo -e "\033[0;31mLa contraseña no puede estar vacía.\033[0m"
        read -s -p "Contraseña para Usuario Root: " ROOT_PASS
        echo ""
    done

    ROOT_HASH=$(echo -n "$ROOT_PASS" | sha256sum | awk '{print $1}')

    cat <<EOF > backend/config/.env
APP_NAME=$APP_NAME
DB_HOST=$DB_HOST
DB_PORT=$DB_PORT
DB_USER=$DB_USER
DB_PASS=$DB_PASS
DB_NAME=$DB_NAME
DB_PREFIX=$PREFIX
EOF

    cat <<EOF > backend/queries/sp_catalog.conf
SP_GET_USER_BY_NAME=sp_users_get_by_name
SP_GET_USER_BY_ID=sp_users_get_by_id
SP_CHECK_USER_EXISTS=sp_users_check_exists
SP_REGISTER_USER=sp_users_register
SP_CHECK_EMAIL_EXISTS=sp_users_check_email
SP_UPDATE_USER_PASSWORD=sp_users_update_password
SP_UPDATE_USER_ROLE=sp_users_update_role
SP_USER_LOGIN=sp_users_login
SP_GET_USERS_LIST=sp_users_get_all
SP_GET_USER_ROLE=sp_users_get_role
SP_GET_ALL_ROLES=sp_obtener_todos_roles
SP_INSERT_ROLE=sp_insertar_rol
SP_EDIT_ROLE=sp_editar_rol
SP_GET_ROLE_BY_ID=sp_obtener_rol_por_id
EOF

    cat <<EOF > backend/sql/bootstrap.sql
CREATE DATABASE IF NOT EXISTS \`${DB_NAME}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE \`${DB_NAME}\`;

CREATE TABLE IF NOT EXISTS \`${PREFIX}roles\` (
    \`id\` INT AUTO_INCREMENT PRIMARY KEY,
    \`descripcion\` VARCHAR(100) NOT NULL UNIQUE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS \`${PREFIX}usuarios\` (
    \`id\` INT AUTO_INCREMENT PRIMARY KEY,
    \`nombre\` VARCHAR(128) NOT NULL,
    \`email\` VARCHAR(128) NOT NULL UNIQUE,
    \`password\` VARCHAR(128) NOT NULL,
    \`rol_id\` INT NOT NULL DEFAULT 1,
    \`fecha_creacion\` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (\`rol_id\`) REFERENCES \`${PREFIX}roles\`(\`id\`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO \`${PREFIX}roles\` (\`id\`, \`descripcion\`) VALUES (1, 'Administrador Root') ON DUPLICATE KEY UPDATE \`descripcion\`='Administrador Root';
INSERT INTO \`${PREFIX}roles\` (\`id\`, \`descripcion\`) VALUES (2, 'Operador') ON DUPLICATE KEY UPDATE \`descripcion\`='Operador';

INSERT INTO \`${PREFIX}usuarios\` (\`nombre\`, \`email\`, \`password\`, \`rol_id\`)
VALUES ('$ROOT_NAME', '$ROOT_EMAIL', '$ROOT_HASH', 1)
ON DUPLICATE KEY UPDATE \`password\`='$ROOT_HASH';

DELIMITER //

DROP PROCEDURE IF EXISTS \`sp_users_get_by_name\` //
CREATE PROCEDURE \`sp_users_get_by_name\`(IN p_nombre VARCHAR(128))
BEGIN
    SELECT id, nombre, email, rol_id FROM \`${PREFIX}usuarios\` WHERE nombre = p_nombre LIMIT 1;
END //

DROP PROCEDURE IF EXISTS \`sp_users_get_by_id\` //
CREATE PROCEDURE \`sp_users_get_by_id\`(IN p_id INT)
BEGIN
    SELECT id, nombre, email, rol_id FROM \`${PREFIX}usuarios\` WHERE id = p_id LIMIT 1;
END //

DROP PROCEDURE IF EXISTS \`sp_users_check_exists\` //
CREATE PROCEDURE \`sp_users_check_exists\`(IN p_nombre VARCHAR(128), IN p_email VARCHAR(128))
BEGIN
    SELECT COUNT(*) FROM \`${PREFIX}usuarios\` WHERE nombre = p_nombre OR email = p_email;
END //

DROP PROCEDURE IF EXISTS \`sp_users_register\` //
CREATE PROCEDURE \`sp_users_register\`(IN p_nombre VARCHAR(128), IN p_email VARCHAR(128), IN p_password VARCHAR(128), IN p_rol INT)
BEGIN
    INSERT INTO \`${PREFIX}usuarios\` (nombre, email, password, rol_id) VALUES (p_nombre, p_email, p_password, p_rol);
    SELECT LAST_INSERT_ID() AS id;
END //

DROP PROCEDURE IF EXISTS \`sp_users_check_email\` //
CREATE PROCEDURE \`sp_users_check_email\`(IN p_email VARCHAR(128))
BEGIN
    SELECT COUNT(*) FROM \`${PREFIX}usuarios\` WHERE email = p_email;
END //

DROP PROCEDURE IF EXISTS \`sp_users_update_password\` //
CREATE PROCEDURE \`sp_users_update_password\`(IN p_email VARCHAR(128), IN p_password VARCHAR(128))
BEGIN
    UPDATE \`${PREFIX}usuarios\` SET password = p_password WHERE email = p_email;
END //

DROP PROCEDURE IF EXISTS \`sp_users_update_role\` //
CREATE PROCEDURE \`sp_users_update_role\`(IN p_id INT, IN p_rol INT)
BEGIN
    UPDATE \`${PREFIX}usuarios\` SET rol_id = p_rol WHERE id = p_id;
END //

DROP PROCEDURE IF EXISTS \`sp_users_login\` //
CREATE PROCEDURE \`sp_users_login\`(IN p_email VARCHAR(128))
BEGIN
    SELECT id, nombre, email, password, rol_id FROM \`${PREFIX}usuarios\` WHERE email = p_email LIMIT 1;
END //

DROP PROCEDURE IF EXISTS \`sp_users_get_all\` //
CREATE PROCEDURE \`sp_users_get_all\`()
BEGIN
    SELECT u.id, u.nombre, u.email, r.descripcion AS rol
    FROM \`${PREFIX}usuarios\` u
    LEFT JOIN \`${PREFIX}roles\` r ON u.rol_id = r.id;
END //

DROP PROCEDURE IF EXISTS \`sp_users_get_role\` //
CREATE PROCEDURE \`sp_users_get_role\`(IN p_id INT)
BEGIN
    SELECT rol_id FROM \`${PREFIX}usuarios\` WHERE id = p_id LIMIT 1;
END //

DROP PROCEDURE IF EXISTS \`sp_obtener_todos_roles\` //
CREATE PROCEDURE \`sp_obtener_todos_roles\`()
BEGIN
    SELECT id, descripcion FROM \`${PREFIX}roles\`;
END //

DROP PROCEDURE IF EXISTS \`sp_insertar_rol\` //
CREATE PROCEDURE \`sp_insertar_rol\`(IN p_descripcion VARCHAR(100))
BEGIN
    INSERT INTO \`${PREFIX}roles\` (descripcion) VALUES (p_descripcion);
    SELECT LAST_INSERT_ID() AS id;
END //

DROP PROCEDURE IF EXISTS \`sp_editar_rol\` //
CREATE PROCEDURE \`sp_editar_rol\`(IN p_id INT, IN p_descripcion VARCHAR(100))
BEGIN
    UPDATE \`${PREFIX}roles\` SET descripcion = p_descripcion WHERE id = p_id;
END //

DROP PROCEDURE IF EXISTS \`sp_obtener_rol_por_id\` //
CREATE PROCEDURE \`sp_obtener_rol_por_id\`(IN p_id INT)
BEGIN
    SELECT id, descripcion FROM \`${PREFIX}roles\` WHERE id = p_id LIMIT 1;
END //

DELIMITER ;
EOF

    echo "Ejecutando bootstrap SQL..."
    if [ -z "$DB_PASS" ]; then
        mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" < backend/sql/bootstrap.sql
    else
        mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" < backend/sql/bootstrap.sql
    fi

    if [ $? -eq 0 ]; then
        echo -e "\033[0;32m✅ Base de datos e infraestructura creadas exitosamente.\033[0m"
    else
        echo -e "\033[0;31m❌ Error ejecutando bootstrap.sql. Verifique sus credenciales.\033[0m"
        exit 1
    fi

    echo -e "\n[4/4] Iniciando '$APP_NAME' en Entorno de Desarrollo..."

    APP_URL="http://localhost:5001/"

    open_browser() {
        local url=$1
        sleep 1.5
        if command -v xdg-open &> /dev/null; then
            xdg-open "$url" &> /dev/null &
        elif command -v sensible-browser &> /dev/null; then
            sensible-browser "$url" &> /dev/null &
        elif command -v x-www-browser &> /dev/null; then
            x-www-browser "$url" &> /dev/null &
        elif command -v firefox &> /dev/null; then
            firefox "$url" &> /dev/null &
        elif command -v google-chrome &> /dev/null; then
            google-chrome "$url" &> /dev/null &
        fi
    }

    open_browser "$APP_URL"
    cd backend/bin/
    chassisCore
    ;;

fi
