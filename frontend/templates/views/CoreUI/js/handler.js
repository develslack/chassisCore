/**
 * chassisCore - handler.js
 * Motor Core UI de enrutamiento SPA con URLs limpias (History API).
 */

const CoreUI = (function() {
    let config = {
        viewContainerId: 'app-view',
        viewsFolder: 'views/logic/components/',
        defaultView: 'login',
        baseRoute: '/',            // Ruta base limpia ('/' o '/dashboard')
        preserveBaseRoute: false   // En 'true', fija la URL en baseRoute al navegar subcomponentes
    };

    let businessLogic = null;

    function init(userConfig = {}, appLogicModule = null) {
        config = Object.assign({}, config, userConfig);
        businessLogic = appLogicModule;

        // 1. Limpiar .html de la barra de direcciones si se ingresó directamente
        cleanAddressBar();

        // 2. Escuchar clics globales en elementos con data-route
        document.addEventListener('click', function(e) {
            const target = e.target.closest('[data-route]');
            if (target) {
                e.preventDefault();
                const viewName = target.getAttribute('data-route');
                navigateTo(viewName);
            }
        });

        // 3. Manejar botones del navegador (Atrás / Adelante)
        window.addEventListener('popstate', function(e) {
            const viewName = (e.state && e.state.view) ? e.state.view : getRouteFromUrl() || config.defaultView;
            loadView(viewName, true);
        });

        // 4. Cargar la vista inicial
        const initialView = config.preserveBaseRoute ? config.defaultView : (getRouteFromUrl() || config.defaultView);
        loadView(initialView, true);
    }

    /**
     * Limpia la barra de direcciones eliminando sufijos .html
     */
    function cleanAddressBar() {
        let path = window.location.pathname;

        if (path.endsWith('/index.html')) {
            path = path.replace(/\/index\.html$/, '/') || '/';
            history.replaceState({ view: config.defaultView }, '', path);
        } else if (path.endsWith('/dashboard.html')) {
            path = path.replace(/\/dashboard\.html$/, '/dashboard');
            history.replaceState({ view: config.defaultView }, '', path);
        }
    }

    /**
     * Extrae el nombre de la vista desde la URL pública
     */
    function getRouteFromUrl() {
        let path = window.location.pathname;
        path = path.replace(/\.html$/, '');
        const cleanPath = path.replace(/^\/|\/$/g, '');

        if (!cleanPath || cleanPath === 'dashboard') {
            return null;
        }
        return cleanPath;
    }

    /**
     * Cambia la vista e inyecta el HTML parcial sin ensuciar la URL
     */
    function navigateTo(viewName) {
        let newUrl = config.baseRoute;

        if (!config.preserveBaseRoute) {
            // Modo Auth: Muestra URLs limpias como /login, /registro, /password
            newUrl = (viewName === config.defaultView) ? config.baseRoute : `/${viewName}`;
        }

        // Actualizar URL solo si cambia
        if (window.location.pathname !== newUrl) {
            history.pushState({ view: viewName }, '', newUrl);
        }

        loadView(viewName, true);
    }

    /**
     * Carga asíncrona del componente parcial desde views/logic/components/
     */
     async function loadView(viewName, triggerHook = true) {
        const container = document.getElementById(config.viewContainerId);
        if (!container) return;

        try {
            // Transición de salida rápida
            container.style.opacity = '0';
            container.style.transform = 'translateY(6px)';
            container.style.transition = 'opacity 0.12s ease, transform 0.12s ease';

            const response = await fetch(`${config.viewsFolder}${viewName}.html`);
            if (!response.ok) throw new Error(`HTTP error ${response.status}`);

            const htmlContent = await response.text();

            setTimeout(() => {
                container.innerHTML = htmlContent;

                // Transición de entrada con leve desplazamiento hacia arriba
                container.style.opacity = '1';
                container.style.transform = 'translateY(0)';

                sessionStorage.setItem('chassis_current_view', viewName);
                updateActiveNavLinks(viewName);

                if (triggerHook && businessLogic && typeof businessLogic.onViewLoaded === 'function') {
                    businessLogic.onViewLoaded(viewName);
                }
            }, 120);

        } catch (error) {
            console.error(`[CoreUI] Error al cargar la vista '${viewName}':`, error);
        }
    }

    /**
     * Mantiene sincronizada la clase .active en botones y enlaces del sidebar/navbar
     */
    function updateActiveNavLinks(viewName) {
        document.querySelectorAll('[data-route]').forEach(link => {
            if (link.getAttribute('data-route') === viewName) {
                link.classList.add('active');
            } else {
                link.classList.remove('active');
            }
        });
    }

    return {
        init: init,
        navigate: navigateTo
    };
})();
