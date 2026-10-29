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
  * Persistencia de tema por usuario individual en `frontend/theme_config/[usuario]-theme.conf` gestionado por `theme_handler`.
* **Asistente Inteligente de Inicialización (`init_project.sh`):**
  * Auditoría automática del sistema host (GCC, OpenSSL, MariaDB/MySQL).
  * Instalación autónoma mediante `sudo` de librerías faltantes (`conio.h` desde la carpeta local `conio_lib/`).
  * **Modo Dual:** Exploración inmediata de la versión precompilada **DEMO** (sin necesidad de configurar base de datos) o **Scaffolding de Nueva App** (generación de proyecto espejo, compilación personalizada, bootstrap de base de datos SQL y usuario `root`).
  * Autodetección del navegador predeterminado del sistema operativo para abrir automáticamente la aplicación al iniciar en `http://localhost:5001/dashboard`.

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

## 🔒 Arquitectura de Base de Datos (Blackbox DAO)

En la capa de lógica de negocio (`*_service.c`), no se escriben consultas SQL ni se referencian tablas directamente. Las interacciones se realizan exclusivamente llamando al identificador del Stored Procedure registrado en el catálogo:

```c
// Ejemplo en users_service.c
const char *params[] = { username, password_hash };
DB_DataSet ds;

if (db_call_sp("SP_USER_LOGIN", params, 2, &ds) == 0) {
    // Procesar dataset agnóstico ds.data[fila][columna]
    db_free_dataset(&ds);
}
```

Las tablas físicas se resuelven dinámicamente mediante el prefijo configurado en el `.env`, aislando completamente la estructura ante accesos o ataques externos.