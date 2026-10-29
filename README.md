# chassisCore Framework

**chassisCore** es un motor de desarrollo *full-stack* ultra ligero enfocado en el alto rendimiento, diseñado para construir aplicaciones **Single Page Application (SPA)** mediante un frontend modular CSS/JS y un backend de alto rendimiento escrito en **C puro (C11)**.

El marco cuenta con una **arquitectura de persistencia agnóstica tipo *Blackbox*** desacoplada de la lógica de negocio mediante variables de entorno (`.env`), introspección de esquema y catálogo de Stored Procedures, acompañado de un asistente interactivo de inicialización (`init_project.sh`).

---

## 🚀 Características Principales

* **Backend Ultra Rápido en C (C11):** Servidor HTTP embebido en C nativo con soporte para múltiples hilos (`pthread`), gestión modular de rutas y sesiones en memoria RAM.
* **Capa de Persistencia Blackbox (Agnóstica y Segura):**
  * Carga dinámica de credenciales desde `backend/config/.env`.
  * Introspección automática de esquema e inferencia de prefijos de tabla (`db_schema`).
  * Catálogo de consultas desacoplado mediante Stored Procedures (`sp_catalog.conf`), evitando consultas SQL directamente escritas en la lógica de negocio C.
  * Estructura neutra de datos `DB_DataSet` para intercambio agnóstico de resultados.
* **Engine Frontend & Motor UI Modular:**
  * Componentes dinámicos: Tablas con paginación/exportación, loaders, relojes de espera, animaciones avanzadas (*Skeletons* y *Ripples*).
  * **Sistema de 12 Temas Visuales:** Incluye temas retro/modernos de SO (destacando el tema clásico **Ubuntu Ambiance Darkness GTK3**).
  * Persistencia de tema por usuario individual en `frontend/theme_config/[usuario]-theme.conf` mediante `theme_handler`.
* **Asistente Inteligente de Inicialización (`init_project.sh`):**
  * Auditoría automática del sistema host (GCC, OpenSSL, MariaDB/MySQL).
  * Instalación autónoma mediante `sudo` de librerías faltantes (`conio.h` desde la carpeta local `conio_lib/`).
  * **Modo Dual:** Exploración inmediata de la versión precompilada **DEMO** (sin necesidad de configurar base de datos) o **Scaffolding de Nueva App** (generación de proyecto espejo, compilación personalizada, bootstrap de base de datos SQL y usuario `root`).
  * Autodetección del navegador predeterminado del sistema operativo para abrir automáticamente la aplicación en `http://localhost:5001/dashboard`.

---

## 🛠️ Requisitos del Sistema

Para compilar y ejecutar **chassisCore**, el sistema host (Linux/Debian/Slackware/Ubuntu/Fedora/Arch) debe contar con:

* **Compilador C:** `gcc` (soporte C11/gnu11).
* **Librerías C:** `libssl-dev` / `crypto` (OpenSSL), `libmysqlclient-dev` (MariaDB/MySQL Client), `pthread`, `lm`.
* **Motor de Base de Datos (Opcional para modo DEMO):** Servidor local o remoto MariaDB / MySQL activo.

> **Nota:** Si la librería `conio.h` no se encuentra en tu sistema, el script de inicio la instalará automáticamente desde la carpeta local `conio_lib/`.

---

## 📦 Estructura del Proyecto

```text
chassisCore/
├── backend/
│   ├── bin/                  # Ejecutables compilados (chassisCore)
│   ├── config/               # Archivo .env (Credenciales e IP)
│   ├── queries/              # sp_catalog.conf (Mapeo de Stored Procedures)
│   ├── sql/                  # bootstrap.sql (Autogenerado durante el setup)
│   └── src/
│       ├── bussiness/        # Lógica de negocio (users_service, roles_service)
│       └── system/           # Core del sistema (db, auth, theme_handler, routes, session_manager)
├── conio_lib/                # Archivos fuente de respaldo para la librería conio.h / libconia.a
├── frontend/
│   ├── theme_config/         # Archivos de configuración de tema por usuario (*-theme.conf)
│   └── templates/views/
│       ├── CoreUI/           # CSS (themes, animations, loaders) y Assets
│       └── logic/            # JavaScript modular (libcore.js, tables.js) y componentes HTML
├── init_project.sh           # Asistente interactivo principal de inicio
└── compile.sh                # Script alternativo de compilación directa
```

---

## ⚡ Instalación y Puesta en Marcha

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com/develslack/chassisCore.git
   cd chassisCore
   ```

2. **Dar permisos de ejecución al asistente:**
   ```bash
   chmod +x init_project.sh
   ```

3. **Ejecutar el asistente interactivo:**
   ```bash
   ./init_project.sh
   ```

---

## 🎯 Modos de Ejecución en `init_project.sh`

Al ejecutar `./init_project.sh`, el script realizará la auditoría de dependencias y te presentará las siguientes opciones:

### 1. Modo DEMO (`[1] Probar la App DEMO`)
* **Uso:** Ideal para explorar la UI, componentes, animaciones y cambio de temas inmediatamente.
* **Sin Base de Datos:** Corre directamente el ejecutable precompilado `backend/bin/chassisCore` sin solicitar credenciales ni modificar MySQL.
* **Resultado:** Abre automáticamente el navegador en `http://localhost:5001/dashboard`.

### 2. Modo NUEVA APP (`[2] Crear una NUEVA Aplicación`)
* **Scaffolding Espejo:** Te solicitará el nombre de tu nueva aplicación (ej: `Music Center`) y clonará el framework a un directorio hermano limpio (`../music_center/`) sin archivos `.git`.
* **Configuración de Persistencia:** Solicitará las credenciales de tu base de datos y creará automáticamente:
  * El archivo `backend/config/.env` con el prefijo autogenerado de tablas (ej: `mc_`).
  * El archivo `backend/queries/sp_catalog.conf`.
  * La base de datos, tablas principales (`usuarios`, `roles`) y Stored Procedures mediante `bootstrap.sql`.
* **Creación de Superusuario:** Obligará a registrar las credenciales del usuario `root` inicial.
* **Compilación y Lanzamiento:** Compila el binario personalizado `backend/bin/music_center` e inicia el servidor en `http://localhost:5001/dashboard`.

---

## 📚 Documentación para Desarrolladores - BACKEND

### Arquitectura General

El backend sigue un modelo modular dividido en dos capas bien definidas dentro de `backend/src/`:

1. **Capa del Sistema (`src/system/`):** Provee la infraestructura de bajo nivel (servidores HTTP, enrutamiento, seguridad, gestión de memoria y persistencia Blackbox).
2. **Capa de Negocio (`src/bussiness/`):** Implementa las reglas de dominio mediante servicios (`*_service.c`). Esta capa **nunca ejecuta SQL directo** ni conoce los nombres físicos de las tablas o controladores de base de datos.

### Componentes de Infraestructura (`src/system/`)

* **`main.c`**: Coordina la inicialización. Ejecuta la verificación de hardware Fail-Fast (`system_verificar_y_mapear_hardware`), instala los manejadores de señales Unix (`system_blindar_senales`), arranca el administrador de sesiones (`session_manager_init`), inicializa el motor de base de datos (`db_init`), registra las rutas del negocio y enciende los sockets HTTP.
* **`db.h` / `db.c` (Blackbox DAO)**: Administra la conexión MariaDB/MySQL. Ejecuta las llamadas a procedimientos almacenados (`db_call_sp`) con sanidad contra inyección SQL. Mapea las respuestas de la base de datos hacia la estructura neutra `DB_DataSet`, aislando la librería de MySQL de la lógica de aplicación.
* **`env_loader.h` / `env_loader.c`**: Parsea el archivo `backend/config/.env` para cargar host, puerto, usuario, clave, nombre de BD y prefijo dinámico sin requerir recompilación.
* **`db_catalog.h` / `db_catalog.c`**: Carga el catálogo `backend/queries/sp_catalog.conf`. Mapea identificadores lógicos (ej: `SP_USER_LOGIN`) hacia los nombres de procedimientos almacenados reales (ej: `sp_users_login`).
* **`db_schemas.h` / `db_schemas.c`**: Ejecuta la introspección de esquema consultando `INFORMATION_SCHEMA.TABLES`. Extrae y remueve el prefijo configurado para mapear nombres lógicos de tabla en memoria.
* **`backend_server.h` / `backend_server.c`**: Servidor API REST multi-hilo (`pthread`) que escucha peticiones en el puerto `5000` y deriva el cuerpo HTTP hacia `routes.c`.
* **`frontend_server.h` / `frontend_server.c`**: Servidor HTTP embebido en el puerto `5001` dedicado a servir archivos estáticos (HTML, CSS, JS) para la Single Page Application.
* **`routes.h` / `routes.c`**: Enrutador que mapea pares `(MÉTODO, URI)` hacia callbacks handler en C. Provee la API `send_response` para enviar respuestas HTTP con formato JSON.
* **`session_manager.h` / `session_manager.c`**: Kernel de sesiones en RAM. Genera y valida tokens alfanuméricos de 128 bytes reteniendo en memoria la información de usuario activo.
* **`ArrayList.h` / `ArrayList.c`**: Estructura de lista dinámica basada en punteros genéricos `void*` utilizada para la gestión de cachés en RAM.
* **`theme_handler.h` / `theme_handler.c`**: Manejador del sistema que atiende los endpoints `/theme/get` y `/theme/set`, persistiendo la preferencia visual por usuario en `frontend/theme_config/[usuario]-theme.conf`.
* **`hash.h` / `hash.c`**: Genera hashes criptográficos SHA-256 para la autenticación de usuarios.

### Componentes de Lógica de Negocio (`src/bussiness/`)

* **`users_service.h` / `users_service.c`**: Procesa la lógica de autenticación, registro, recuperación de clave y roles. Invoca la persistencia mediante `db_call_sp` utilizando únicamente claves del catálogo (`SP_USER_LOGIN`, `SP_REGISTER_USER`, etc.) y registra sesiones activas en la RAM del sistema.
* **`roles_service.h` / `roles_service.c`**: Administra la entidad de roles mediante una estrategia de sincronización dual: carga los registros en RAM mediante `ArrayList` al iniciar el sistema y sincroniza instantáneamente las altas y ediciones tanto en memoria como en la base de datos a través de `db_call_sp("SP_INSERT_ROLE")` y `db_call_sp("SP_EDIT_ROLE")`.

---

## 🔒 Patrón de Invocación Blackbox

Toda consulta de datos en los servicios se realiza a través de `db_call_sp`:

```c
// Ejemplo en users_service.c
const char *params[] = { decoded_email };
DB_DataSet ds;

if (db_call_sp("SP_USER_LOGIN", params, 1, &ds) == 0 && ds.num_rows > 0) {
    // ds.data[0][0] -> ID
    // ds.data[0][1] -> Nombre
    // ds.data[0][3] -> Password Hash
    db_free_dataset(&ds);
}
```

Las estructuras `DB_DataSet` liberan su memoria asignada explícitamente con `db_free_dataset(&ds)`, garantizando cero fugas de memoria (*memory leaks*) en el backend.

---

## 🎨 Documentación para Desarrolladores - FRONTEND

### Arquitectura de Interfaz SPA

El frontend de **chassisCore** es una Single Page Application (SPA) modular escrita en **JavaScript ES6+ nativo y CSS3 puro**. No utiliza librerías externas ni herramientas de compilación/bundling.

Es servido directamente por el ejecutable C a través de `frontend_server.c` en el puerto `5001` y consume los servicios API REST exponiendo respuestas JSON en el puerto `5000`.

### Componentes y Estructura (`frontend/templates/views/CoreUI/`)

* **`logic/components/libcore.js` (Kernel Frontend)**:
  * Controla la navegación del cliente (Router SPA sin recarga de página).
  * Gestiona los tokens de sesión de 128 bytes y las cabeceras HTTP de autenticación.
  * Provee las funciones `syncUserTheme(username)` y `saveUserTheme(username, theme)` para interactuar con los endpoints del backend `/theme/get` y `/theme/set`.

* **`logic/components/tables.js` (Motor de Tablas Dinámicas)**:
  * Procesa conjuntos de datos neutros (`DB_DataSet`) devueltos en JSON por las APIs.
  * Construye tablas HTML enriquecidas con paginación en tiempo real, buscador global instantáneo, ordenación por encabezados y botones de exportación a CSV, PDF y Excel.

* **`css/themes.css` (Engine de 12 Temas Visuales)**:
  * Almacena las definiciones CSS3 basadas en Custom Properties (`--bg-primary`, `--accent-color`, `--text-main`, etc.).
  * Permite la conmutación instantánea del entorno entre 12 temas (incluyendo **Ubuntu Ambiance Darkness GTK3**, **Win11**, **Cyberpunk**, etc.) cambiando el atributo `data-theme` en el documento HTML.

* **`css/animations-advanced.css` & `loaders.css`**:
  * Proporcionan animaciones fluidas de UI: animaciones **Skeleton** durante peticiones asíncronas, efectos **Ripple** en botones y elementos interactivos, e indicadores de espera con spinners y relojes analógicos/digitales animables.

### Sincronización y Persistencia de Temas

1. Al iniciar la SPA, `libcore.js` invoca a `GET /theme/get?user=[usuario]`.
2. El backend (`theme_handler.c`) consulta el archivo de configuración en `frontend/theme_config/[usuario]-theme.conf`.
3. El tema retornado se aplica globalmente en el DOM sin recargar la página web.
4. Cuando el usuario cambia el tema desde el selector de la UI, se invoca `POST /theme/set`. En entorno de desarrollo/producción, la nueva preferencia se guarda en disco; en **Modo DEMO**, la preferencia se aplica únicamente en la memoria de la sesión actual sin alterar archivos en disco.