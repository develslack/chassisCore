/**
 * chassisCore - libcore.js
 * Utilidades globales del núcleo
 */
console.log("LibCore cargado...");

const LibCore = (function() {

    // --- Control de Contraseñas ---
    function togglePassword(inputOrId, triggerBtn = null) {
        const input = typeof inputOrId === 'string' ? document.getElementById(inputOrId) : inputOrId;
        if (!input) return;

        const isPassword = input.type === 'password';
        input.type = isPassword ? 'text' : 'password';

        if (triggerBtn) {
            const iconSpan = triggerBtn.querySelector('.icon');
            if (iconSpan) {
                if (isPassword) {
                    iconSpan.classList.remove('icon-actions-layer-visible-on');
                    iconSpan.classList.add('icon-actions-layer-visible-off');
                } else {
                    iconSpan.classList.remove('icon-actions-layer-visible-off');
                    iconSpan.classList.add('icon-actions-layer-visible-on');
                }
            }
        }
    }


    // ======================================================================================================================================= //


    // --- Control de Modales ---
    function openModal(modalId) {
        const modalEl = typeof modalId === 'string' ? document.getElementById(modalId) : modalId;
        if (!modalEl) return;

        modalEl.classList.add('active');
        document.body.style.overflow = 'hidden';
    }

    function closeModal(modalId) {
        const modalEl = typeof modalId === 'string' ? document.getElementById(modalId) : modalId;
        if (!modalEl) return;

        modalEl.classList.remove('active');
        document.body.style.overflow = '';
    }


    // ======================================================================================================================================= //


    // --- Sistema de Notificaciones (Toasts) ---
    function toast(message, type = 'default', title = '', duration = 4000) {
        let container = document.getElementById('toast-container');
        if (!container) {
            container = document.createElement('div');
            container.id = 'toast-container';
            document.body.appendChild(container);
        }

        const toastEl = document.createElement('div');
        toastEl.className = `toast toast-${type}`;

        const contentEl = document.createElement('div');
        contentEl.className = 'toast-content';

        if (title) {
            const titleEl = document.createElement('div');
            titleEl.className = 'toast-title';
            titleEl.textContent = title;
            contentEl.appendChild(titleEl);
        }

        const msgEl = document.createElement('div');
        msgEl.textContent = message;
        contentEl.appendChild(msgEl);

        const closeBtn = document.createElement('button');
        closeBtn.className = 'toast-close';
        closeBtn.innerHTML = '&times;';
        closeBtn.onclick = () => removeToast(toastEl);

        toastEl.appendChild(contentEl);
        toastEl.appendChild(closeBtn);
        container.appendChild(toastEl);

        if (duration > 0) {
            setTimeout(() => {
                removeToast(toastEl);
            }, duration);
        }
    }

    function removeToast(toastEl) {
        if (!toastEl || toastEl.classList.contains('toast-hiding')) return;
        toastEl.classList.add('toast-hiding');
        toastEl.addEventListener('transitionend', () => {
            if (toastEl.parentNode) {
                toastEl.parentNode.removeChild(toastEl);
            }
        });
    }

    // --- Control de Paneles Plegables ---
    function togglePanel(panelOrId) {
        const panelEl = typeof panelOrId === 'string' ? document.getElementById(panelOrId) : panelOrId;
        if (!panelEl) return;

        panelEl.classList.toggle('panel-collapsed');
    }


    // ======================================================================================================================================= //


    // --- Control de Jumbotrons ---
    function dismissJumbotron(jumbotronOrId) {
        const jumbotronEl = typeof jumbotronOrId === 'string' ? document.getElementById(jumbotronOrId) : jumbotronOrId;
        if (!jumbotronEl) return;

        jumbotronEl.style.transition = 'opacity 0.2s ease, transform 0.2s ease';
        jumbotronEl.style.opacity = '0';
        jumbotronEl.style.transform = 'translateY(-10px)';
        setTimeout(() => {
            if (jumbotronEl.parentNode) {
                jumbotronEl.parentNode.removeChild(jumbotronEl);
            }
        }, 200);
    }

    // --- Control de Alerts Inline ---
    function dismissAlert(alertOrId) {
        const alertEl = typeof alertOrId === 'string' ? document.getElementById(alertOrId) : alertOrId;
        if (!alertEl) return;

        alertEl.style.transition = 'opacity 0.2s ease, transform 0.2s ease';
        alertEl.style.opacity = '0';
        alertEl.style.transform = 'translateY(-5px)';
        setTimeout(() => {
            if (alertEl.parentNode) {
                alertEl.parentNode.removeChild(alertEl);
            }
        }, 200);
    }


    // ======================================================================================================================================= //


    // --- Control de Barras de Progreso ---
    function setProgress(progressOrId, percent, labelText = null) {
        const container = typeof progressOrId === 'string' ? document.getElementById(progressOrId) : progressOrId;
        if (!container) return;

        const bar = container.querySelector('.progress-bar');
        if (!bar) return;

        const clampedPercent = Math.min(100, Math.max(0, percent));
        bar.style.width = `${clampedPercent}%`;

        if (labelText !== null) {
            bar.textContent = labelText;
        } else if (bar.getAttribute('data-show-percent') === 'true') {
            bar.textContent = `${Math.round(clampedPercent)}%`;
        }
    }


    // ======================================================================================================================================= //


    // --- Control de Collapse / Acordeones ---
    function toggleCollapse(targetId, triggerEl = null) {
        const contentEl = typeof targetId === 'string' ? document.getElementById(targetId) : targetId;
        if (!contentEl) return;

        const isShowing = contentEl.classList.contains('show');
        const cardEl = contentEl.closest('.collapse-card');
        const parentId = triggerEl ? triggerEl.getAttribute('data-core-parent') : null;

        if (parentId && !isShowing) {
            const parentEl = document.getElementById(parentId);
            if (parentEl) {
                parentEl.querySelectorAll('.collapse-content.show').forEach(openContent => {
                    openContent.classList.remove('show');
                    const openCard = openContent.closest('.collapse-card');
                    if (openCard) openCard.classList.remove('open');
                });
            }
        }

        if (isShowing) {
            contentEl.classList.remove('show');
            if (cardEl) cardEl.classList.remove('open');
            if (triggerEl) triggerEl.classList.remove('active');
        } else {
            contentEl.classList.add('show');
            if (cardEl) cardEl.classList.add('open');
            if (triggerEl) triggerEl.classList.add('active');
        }
    }


    // ======================================================================================================================================= //


    // --- Control de Pestañas (Tabs) ---
    function switchTab(triggerBtn) {
        if (!triggerBtn) return;

        const targetId = triggerBtn.getAttribute('data-target');
        const navContainer = triggerBtn.closest('.nav-tabs');
        if (!targetId || !navContainer) return;

        // Desactivar todas las pestañas del grupo
        navContainer.querySelectorAll('.nav-tab-item').forEach(btn => btn.classList.remove('active'));
        triggerBtn.classList.add('active');

        // Buscar el contenedor de paneles asociado
        const parentSection = navContainer.parentElement;
        if (parentSection) {
            const tabContentContainer = parentSection.querySelector('.tab-content');
            if (tabContentContainer) {
                // Ocultar todos los paneles
                tabContentContainer.querySelectorAll('.tab-pane').forEach(pane => pane.classList.remove('active'));

                // Mostrar el panel objetivo
                const targetPane = document.getElementById(targetId);
                if (targetPane) targetPane.classList.add('active');
            }
        }
    }


    // ======================================================================================================================================= //

    // Listener para actualizar el nombre de archivo seleccionado en .form-file
    function initFileInputs() {
        document.addEventListener('change', function(e) {
            if (e.target && e.target.classList.contains('form-file-input')) {
                const input = e.target;
                const labelSpan = input.parentNode.querySelector('.form-file-name');
                if (labelSpan) {
                    if (input.files && input.files.length > 0) {
                        labelSpan.textContent = input.files.length === 1 ? input.files[0].name : `${input.files.length} archivos seleccionados`;
                        labelSpan.style.color = '#f3f4f6';
                    } else {
                        labelSpan.textContent = labelSpan.getAttribute('data-default') || 'Seleccionar archivo...';
                        labelSpan.style.color = '#9ca3af';
                    }
                }
            }
        });
    }

    // ======================================================================================================================================= //

    // --- Control de Popovers ---
    function togglePopover(targetId, triggerEl = null) {
        const popoverEl = typeof targetId === 'string' ? document.getElementById(targetId) : targetId;
        if (!popoverEl) return;

        const isShowing = popoverEl.classList.contains('show');

        // Cerrar cualquier otro popover activo en pantalla
        document.querySelectorAll('.popover.show').forEach(openPop => {
            if (openPop !== popoverEl) openPop.classList.remove('show');
        });

        if (isShowing) {
            popoverEl.classList.remove('show');
        } else {
            popoverEl.classList.add('show');
        }
    }

    function closeAllPopovers() {
        document.querySelectorAll('.popover.show').forEach(pop => pop.classList.remove('show'));
    }

    // Evento de Popovers al hacer clic fuera
    document.addEventListener('click', function(e) {
        // Disparador de Popover
        const popoverTrigger = e.target.closest('[data-core-toggle="popover"]');
        if (popoverTrigger) {
            e.preventDefault();
            e.stopPropagation();
            const targetId = popoverTrigger.getAttribute('data-target');
            if (targetId) togglePopover(targetId, popoverTrigger);
            return;
        }

        // Botón de cierre interno dentro del popover
        const popoverClose = e.target.closest('[data-core-dismiss="popover"]');
        if (popoverClose) {
            const pop = popoverClose.closest('.popover');
            if (pop) pop.classList.remove('show');
            return;
        }

        // Clic fuera de cualquier popover abierto -> Cerrar todos
        if (!e.target.closest('.popover')) {
            closeAllPopovers();
        }
    });


    // ======================================================================================================================================= //


    // --- Control de Indicadores de Carga ---
    function showLoader(targetOrId, type = 'spinner', color = 'submit', message = '') {
        const targetEl = typeof targetOrId === 'string' ? document.getElementById(targetOrId) : targetOrId;
        if (!targetEl) return;

        // Remover overlay previo si ya existía
        hideLoader(targetEl);

        if (getComputedStyle(targetEl).position === 'static') {
            targetEl.style.position = 'relative';
        }

        const overlay = document.createElement('div');
        overlay.className = 'loader-overlay';

        let loaderHtml = '';
        if (type === 'dots') {
            loaderHtml = `<div class="loader-dots loader-${color}"><span></span><span></span><span></span></div>`;
        } else {
            loaderHtml = `<div class="loader loader-${type} loader-md loader-${color}"></div>`;
        }

        if (message) {
            loaderHtml += `<div class="loader-text">${message}</div>`;
        }

        overlay.innerHTML = loaderHtml;
        targetEl.appendChild(overlay);
    }

    function hideLoader(targetOrId) {
        const targetEl = typeof targetOrId === 'string' ? document.getElementById(targetOrId) : targetOrId;
        if (!targetEl) return;

        const overlay = targetEl.querySelector('.loader-overlay');
        if (overlay) {
            overlay.remove();
        }
    }


    // ======================================================================================================================================= //


    // --- Control de Animaciones y Visibilidad ---
    function showElement(elementOrId, animationClass = 'fade-in') {
        const el = typeof elementOrId === 'string' ? document.getElementById(elementOrId) : elementOrId;
        if (!el) return;

        el.classList.remove('is-hidden', 'fade-out', 'zoom-out', 'slide-up');
        el.classList.add('is-visible', animationClass);
    }

    function hideElement(elementOrId, animationClass = 'fade-out') {
        const el = typeof elementOrId === 'string' ? document.getElementById(elementOrId) : elementOrId;
        if (!el) return;

        el.classList.remove('fade-in', 'zoom-in', 'slide-down', 'slide-right');
        el.classList.add(animationClass);

        el.addEventListener('animationend', function handler() {
            el.classList.add('is-hidden');
            el.classList.remove('is-visible', animationClass);
            el.removeEventListener('animationend', handler);
        }, { once: true });
    }

    function shakeElement(elementOrId) {
        const el = typeof elementOrId === 'string' ? document.getElementById(elementOrId) : elementOrId;
        if (!el) return;

        el.classList.add('shake');
        el.addEventListener('animationend', function handler() {
            el.classList.remove('shake');
            el.removeEventListener('animationend', handler);
        }, { once: true });
    }


    // ======================================================================================================================================= //


    // --- Inicialización del Efecto Ripple ---
    function initRippleEffect() {
        document.addEventListener('click', function(e) {
            const target = e.target.closest('.btn, .ripple-target');
            if (!target) return;

            const rect = target.getBoundingClientRect();
            const circle = document.createElement('span');
            const diameter = Math.max(rect.width, rect.height);
            const radius = diameter / 2;

            circle.style.width = circle.style.height = `${diameter}px`;
            circle.style.left = `${e.clientX - rect.left - radius}px`;
            circle.style.top = `${e.clientY - rect.top - radius}px`;
            circle.classList.add('core-ripple');

            const existingRipple = target.querySelector('.core-ripple');
            if (existingRipple) existingRipple.remove();

            target.appendChild(circle);

            circle.addEventListener('animationend', () => circle.remove());
        });
    }

    // --- Gestión de Skeletons ---
    function renderSkeletonCard(containerOrId) {
        const el = typeof containerOrId === 'string' ? document.getElementById(containerOrId) : containerOrId;
        if (!el) return;

        el.innerHTML = `
            <div class="skeleton-card">
                <div class="skeleton skeleton-text skeleton-text-title"></div>
                <div class="skeleton skeleton-text"></div>
                <div class="skeleton skeleton-text skeleton-text-short"></div>
                <div style="display: flex; gap: 0.5rem; margin-top: 0.5rem;">
                    <div class="skeleton skeleton-btn"></div>
                    <div class="skeleton skeleton-btn"></div>
                </div>
            </div>`;
    }


    // ======================================================================================================================================= //


    // --- Sistema de Cambio de Temas Dinámicos ---
    function setTheme(themeName) {
        if (!themeName) return;

        document.documentElement.setAttribute('data-theme', themeName);
        localStorage.setItem('chassis_theme', themeName);

        window.dispatchEvent(new CustomEvent('chassisThemeChanged', { detail: { theme: themeName } }));
        console.log(`[ThemeEngine] Tema cambiado a: '${themeName}'`);
    }

    function getTheme() {
        return document.documentElement.getAttribute('data-theme') || localStorage.getItem('chassis_theme') || 'dark';
    }

    function initTheme() {
        const savedTheme = localStorage.getItem('chassis_theme') || 'dark';
        setTheme(savedTheme);
    }


    // ======================================================================================================================================= //


    async function syncUserTheme(username) {
        try {
            const response = await fetch(`/theme/get?user=${encodeURIComponent(username)}`);
            const data = await response.json();
            if (data.status === 'ok' && data.theme) {
                LibCore.setTheme(data.theme);
                const selector = document.getElementById('select-theme-switcher');
                if (selector) selector.value = data.theme;
            }
        } catch (err) {
            console.warn('Error recuperando tema del usuario, aplicando default:', err);
            LibCore.setTheme('ubuntu-ambiance');
        }
    }

    // Guardar la selección del tema visual
    async function saveUserTheme(username, newTheme) {
        LibCore.setTheme(newTheme); // Cambio instantáneo en la UI
        try {
            await fetch('/theme/set', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `user=${encodeURIComponent(username)}&theme=${encodeURIComponent(newTheme)}`
            });
        } catch (err) {
            console.error('Error guardando preferencia de tema:', err);
        }
    }

    // ======================================================================================================================================= //


    // Escuchadores globales
    function initGlobalListeners() {
        initFileInputs();
        initRippleEffect();
        initTheme();

        document.addEventListener('click', function(e) {
            // Disparador de Tabs
            const tabTrigger = e.target.closest('[data-core-toggle="tab"]');
            if (tabTrigger) {
                e.preventDefault();
                switchTab(tabTrigger);
                return;
            }

            // Disparador de Collapse
            const collapseTrigger = e.target.closest('[data-core-toggle="collapse"]');
            if (collapseTrigger) {
                e.preventDefault();
                const targetId = collapseTrigger.getAttribute('data-target');
                if (targetId) toggleCollapse(targetId, collapseTrigger);
                return;
            }

            // Descartar Alert Inline
            const alertDismiss = e.target.closest('[data-core-dismiss="alert"]');
            if (alertDismiss) {
                const alertEl = alertDismiss.closest('.alert');
                if (alertEl) dismissAlert(alertEl);
                return;
            }

            // Descartar Jumbotron
            const jumbotronDismiss = e.target.closest('[data-core-dismiss="jumbotron"]');
            if (jumbotronDismiss) {
                const jumbotron = jumbotronDismiss.closest('.jumbotron');
                if (jumbotron) dismissJumbotron(jumbotron);
                return;
            }

            // Plegar/desplegar panel
            const panelToggleBtn = e.target.closest('[data-core-toggle="panel"]');
            if (panelToggleBtn) {
                const panel = panelToggleBtn.closest('.panel');
                if (panel) togglePanel(panel);
                return;
            }

            // Clics de modales
            const dismissTrigger = e.target.closest('[data-core-dismiss="modal"]');
            if (dismissTrigger) {
                const modal = dismissTrigger.closest('.modal-backdrop');
                if (modal) closeModal(modal);
                return;
            }

            if (e.target.classList.contains('modal-backdrop')) {
                closeModal(e.target);
            }
        });

        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') {
                const activeModal = document.querySelector('.modal-backdrop.active');
                if (activeModal) closeModal(activeModal);
            }
        });
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initGlobalListeners);
    } else {
        initGlobalListeners();
    }

    return {
        togglePassword: togglePassword,
        openModal: openModal,
        closeModal: closeModal,
        toast: toast,
        togglePanel: togglePanel,
        dismissJumbotron: dismissJumbotron,
        dismissAlert: dismissAlert,
        setProgress: setProgress,
        toggleCollapse: toggleCollapse,
        switchTab: switchTab,
        showLoader: showLoader,
        hideLoader: hideLoader,
        showElement: showElement,
        hideElement: hideElement,
        shakeElement: shakeElement,
        initRippleEffect: initRippleEffect,
        renderSkeletonCard: renderSkeletonCard,
        setTheme: setTheme,
        getTheme: getTheme,
        initTheme: initTheme
    };
})();
