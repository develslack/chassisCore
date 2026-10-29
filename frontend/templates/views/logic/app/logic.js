/**
 * chassisCore - logic.js
 * Módulo troncal de lógica de negocio (específica de cada app).
 * Se comunica exclusivamente con el CoreUI Engine (`handler.js`).
 */

const AppLogic = {
    /**
     * Hook que se ejecuta automáticamente cada vez que CoreUI carga una vista.
     * @param {string} viewName - El nombre de la vista que se acaba de inyectar en pantalla.
     */
    onViewLoaded: function(viewName) {
        console.log(`[AppLogic] Vista cargada y activa: '${viewName}'`);

        // Auto-inicializar tablas dinámicas si el módulo CoreTable está cargado
        if (typeof CoreTable !== 'undefined' && typeof CoreTable.autoInit === 'function') {
            CoreTable.autoInit();
        }

        // Despachamos la lógica específica según la vista que esté activa
        switch(viewName) {
            case 'login':
                this.initLogin();
                break;
            case 'registro':
                this.initRegistro();
                break;
            case 'recupero':
                this.initRecupero();
                break;
            case 'buttons-demo':
                this.initButtonsDemo();
                break;
            case 'modals-demo':
                this.initModalsDemo();
                break;
            case 'panels-demo':
                this.initPanelsDemo();
                break;
            case 'jumbotron-demo':
                this.initJumbotronDemo();
                break;
            case 'alerts-demo':
                this.initAlertsDemo();
                break;
            case 'badges-demo':
                this.initBadgesDemo();
                break;
            case 'progress-demo':
                this.initProgressDemo();
                break;
            case 'list-groups-demo':
                this.initListGroupsDemo();
                break;
            case 'collapse-demo':
                this.initCollapseDemo();
                break;
            case 'tabs-demo':
                this.initTabsDemo();
                break;
            case 'inputs-demo':
                this.initInputsDemo();
                break;
            case 'tooltips-demo':
                this.initTooltipsDemo();
                break;
            case 'popovers-demo':
                this.initPopoversDemo();
                break;
            case 'tables-demo':
                this.initTablesDemo();
                break;
            case 'loaders-demo':
                this.initLoadersDemo();
                break;
            case 'animations-demo':
                this.initAnimationsDemo();
                break;
            case 'animations-advanced-demo':
                this.initAnimationsAdvancedDemo();
                break;
            case 'themes-demo':
                this.initThemesDemo();
                break;
            default:
                // Si es una vista del dashboard (ej. home, perfil, etc.)
                console.log(`[AppLogic] Vista '${viewName}' sin lógica específica requerida.`);
                break;
        }
    },

    /**
     * Lógica para el formulario de Login
     */
    initLogin: function() {
        console.log("[Módulo] Inicializando eventos de Login...");

        const formLogin = document.getElementById('form-login');
        if (formLogin) {
            formLogin.addEventListener('submit', function(e) {
                e.preventDefault();
                console.log("Formulario de login interceptado. Aquí puedes hacer tu fetch() al backend en C.");
            });
        }
    },

    /**
     * Lógica para el formulario de Registro
     */
    initRegistro: function() {
        console.log("[Módulo] Inicializando eventos de Registro...");
    },

    /**
     * Lógica para el recupero de contraseña
     */
    initRecupero: function() {
        console.log("[Módulo] Inicializando eventos de Recupero de Password...");
    },

    initButtonsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Buttons Demo...");
    },
    initModalsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Modals Demo...");
    },
    initPanelsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Panels Demo...");
    },
    initJumbotronDemo: function() {
        console.log("[Módulo] Inicializando eventos de Jumbotron Demo...");
    },
    initAlertsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Alerts Demo...");
    },
    initBadgesDemo: function() {
        console.log("[Módulo] Inicializando eventos de Badges Demo...");
    },
    initProgressDemo: function() {
        console.log("[Módulo] Inicializando eventos de Progress Demo...");
    },
    initListGroupsDemo: function() {
        console.log("[Módulo] Inicializando eventos de List-Groups Demo...");
    },
    initCollapseDemo: function() {
        console.log("[Módulo] Inicializando eventos de Collapse Demo...");
    },
    initTabsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Tabs Demo...");
    },
    initInputsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Inputs Demo...");
    },
    initTooltipsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Tooltips Demo...");
    },
    initPopoversDemo: function() {
        console.log("[Módulo] Inicializando eventos de PopOvers Demo...");
    },
    initLoadersDemo: function() {
        console.log("[Módulo] Inicializando eventos de Loaders Demo...");
    },
    initAnimationsDemo: function() {
        console.log("[Módulo] Inicializando eventos de Animations Demo...");
    },
    initAnimationsAdvancedDemo: function() {
        console.log("[Módulo] Inicializando eventos de Animations Advanced Demo...");
    },
    initThemesDemo: function() {
        console.log("[Módulo] Inicializando eventos de Themes Demo...");
    },
    initTablesDemo: function() {
        console.log("[Módulo] Inicializando eventos de Tables Demo...");
        if (typeof CoreTable !== 'undefined' && typeof CoreTable.autoInit === 'function') {
            CoreTable.autoInit();
    }

    },
};
