/**
 * chassisCore - tables.js
 * Motor independiente de DataTables con Auto-Inicialización para SPA.
 */
console.log("CoreTable cargado...");

const CoreTable = (function() {

    const instances = new Map();

    class DataTableEngine {
        constructor(tableElement, options = {}) {
            this.table = typeof tableElement === 'string' ? document.getElementById(tableElement) : tableElement;
            if (!this.table) return;

            this.options = Object.assign({
                pageSize: 10,
                pageSizes: [5, 10, 25, 50, 100],
                searchable: true,
                sortable: true,
                exportable: true,
                filename: 'export_data'
            }, options);

            this.currentPage = 1;
            this.sortColumn = -1;
            this.sortDirection = 'asc';
            this.searchQuery = '';

            this.originalRows = [];
            this.filteredRows = [];

            this.init();
        }

        init() {
            this.container = this.table.closest('.table-container') || this.wrapTable();
            this.extractData();
            this.buildToolbar();
            this.buildPagination();
            this.bindEvents();
            this.render();
        }

        wrapTable() {
            const wrapper = document.createElement('div');
            wrapper.className = 'table-container';
            this.table.parentNode.insertBefore(wrapper, this.table);

            const responsiveDiv = document.createElement('div');
            responsiveDiv.className = 'table-responsive';
            responsiveDiv.appendChild(this.table);

            wrapper.appendChild(responsiveDiv);
            return wrapper;
        }

        extractData() {
            const tbody = this.table.querySelector('tbody');
            if (!tbody) return;

            const rows = Array.from(tbody.querySelectorAll('tr'));
            this.originalRows = rows.map((tr, index) => {
                const cells = Array.from(tr.children).map(td => td.innerText.trim());
                return {
                    element: tr,
                    data: cells,
                    index: index
                };
            });

            this.filteredRows = [...this.originalRows];
        }

        buildToolbar() {
            let toolbar = this.container.querySelector('.table-toolbar');
            if (!toolbar) {
                toolbar = document.createElement('div');
                toolbar.className = 'table-toolbar';
                this.container.insertBefore(toolbar, this.container.firstChild);
            }

            let leftHtml = '';
            if (this.options.pageSizes && this.options.pageSizes.length > 0) {
                leftHtml += `
                    <div class="table-toolbar-left">
                        <label class="form-label" style="margin: 0; font-size: 0.75rem;">Mostrar</label>
                        <select class="form-select table-length-select">
                            ${this.options.pageSizes.map(size => `<option value="${size}" ${size == this.options.pageSize ? 'selected' : ''}>${size}</option>`).join('')}
                        </select>
                        <span style="font-size: 0.75rem; color: #9ca3af;">registros</span>
                    </div>`;
            }

            let rightHtml = '<div class="table-toolbar-right">';
            if (this.options.searchable) {
                rightHtml += `<input type="text" class="form-control table-search-input" placeholder="Buscar registro...">`;
            }

            if (this.options.exportable) {
                rightHtml += `
                    <button class="btn btn-default btn-sm btn-export-csv" type="button" title="Exportar CSV">CSV</button>
                    <button class="btn btn-success btn-sm btn-export-excel" type="button" title="Exportar Excel">Excel</button>
                    <button class="btn btn-danger btn-sm btn-export-pdf" type="button" title="Imprimir / PDF">PDF</button>`;
            }
            rightHtml += '</div>';

            toolbar.innerHTML = leftHtml + rightHtml;
        }

        buildPagination() {
            let pagContainer = this.container.querySelector('.table-pagination');
            if (!pagContainer) {
                pagContainer = document.createElement('div');
                pagContainer.className = 'table-pagination';
                this.container.appendChild(pagContainer);
            }

            pagContainer.innerHTML = `
                <div class="table-info">Mostrando 0 a 0 de 0 registros</div>
                <div class="pagination-nav"></div>`;
        }

        bindEvents() {
            // Buscador
            const searchInput = this.container.querySelector('.table-search-input');
            if (searchInput) {
                searchInput.addEventListener('input', (e) => {
                    this.searchQuery = e.target.value.toLowerCase();
                    this.currentPage = 1;
                    this.filterData();
                });
            }

            // Selector registros por página
            const lengthSelect = this.container.querySelector('.table-length-select');
            if (lengthSelect) {
                lengthSelect.addEventListener('change', (e) => {
                    this.options.pageSize = parseInt(e.target.value, 10);
                    this.currentPage = 1;
                    this.render();
                });
            }

            // Ordenamiento por columnas (Ordenar de A-Z, Z-A, Menor a Mayor)
            const headers = this.table.querySelectorAll('thead th');
            headers.forEach((th, colIndex) => {
                if (this.options.sortable && !th.classList.contains('no-sort')) {
                    th.classList.add('sortable');
                    if (!th.querySelector('.sort-icon')) {
                        th.innerHTML += ' <span class="sort-icon">⇅</span>';
                    }
                    th.onclick = () => this.sort(colIndex);
                }
            });

            // Exportación
            const csvBtn = this.container.querySelector('.btn-export-csv');
            if (csvBtn) csvBtn.onclick = () => this.exportCSV();

            const excelBtn = this.container.querySelector('.btn-export-excel');
            if (excelBtn) excelBtn.onclick = () => this.exportExcel();

            const pdfBtn = this.container.querySelector('.btn-export-pdf');
            if (pdfBtn) pdfBtn.onclick = () => this.exportPDF();
        }

        filterData() {
            if (!this.searchQuery) {
                this.filteredRows = [...this.originalRows];
            } else {
                this.filteredRows = this.originalRows.filter(row => {
                    return row.data.some(cellText => cellText.toLowerCase().includes(this.searchQuery));
                });
            }
            this.render();
        }

        sort(colIndex) {
            if (this.sortColumn === colIndex) {
                this.sortDirection = this.sortDirection === 'asc' ? 'desc' : 'asc';
            } else {
                this.sortColumn = colIndex;
                this.sortDirection = 'asc';
            }

            const headers = this.table.querySelectorAll('thead th');
            headers.forEach((th, idx) => {
                th.classList.remove('sort-asc', 'sort-desc');
                const icon = th.querySelector('.sort-icon');
                if (icon) icon.textContent = '⇅';

                if (idx === colIndex) {
                    th.classList.add(this.sortDirection === 'asc' ? 'sort-asc' : 'sort-desc');
                    if (icon) icon.textContent = this.sortDirection === 'asc' ? '▲' : '▼';
                }
            });

            this.filteredRows.sort((a, b) => {
                const valA = a.data[colIndex] || '';
                const valB = b.data[colIndex] || '';

                const numA = parseFloat(valA.replace(/[^0-9.-]+/g, ""));
                const numB = parseFloat(valB.replace(/[^0-9.-]+/g, ""));

                if (!isNaN(numA) && !isNaN(numB)) {
                    return this.sortDirection === 'asc' ? numA - numB : numB - numA;
                }

                return this.sortDirection === 'asc'
                    ? valA.localeCompare(valB, undefined, { numeric: true, sensitivity: 'base' })
                    : valB.localeCompare(valA, undefined, { numeric: true, sensitivity: 'base' });
            });

            this.render();
        }

        render() {
            const tbody = this.table.querySelector('tbody');
            if (!tbody) return;

            tbody.innerHTML = '';

            const totalRows = this.filteredRows.length;
            const totalPages = Math.ceil(totalRows / this.options.pageSize) || 1;

            if (this.currentPage > totalPages) this.currentPage = totalPages;

            const start = (this.currentPage - 1) * this.options.pageSize;
            const end = Math.min(start + this.options.pageSize, totalRows);

            const pageRows = this.filteredRows.slice(start, end);

            if (pageRows.length === 0) {
                tbody.innerHTML = `<tr><td colspan="100%" style="text-align: center; color: #9ca3af; padding: 2rem;">No se encontraron registros.</td></tr>`;
            } else {
                pageRows.forEach(row => tbody.appendChild(row.element));
            }

            const infoEl = this.container.querySelector('.table-info');
            if (infoEl) {
                infoEl.textContent = totalRows > 0
                    ? `Mostrando ${start + 1} a ${end} de ${totalRows} registros`
                    : 'Mostrando 0 a 0 de 0 registros';
            }

            this.renderPaginationButtons(totalPages);
        }

        renderPaginationButtons(totalPages) {
            const nav = this.container.querySelector('.pagination-nav');
            if (!nav) return;

            nav.innerHTML = '';

            const prevBtn = document.createElement('button');
            prevBtn.className = 'page-btn';
            prevBtn.type = 'button';
            prevBtn.innerHTML = '‹';
            prevBtn.disabled = this.currentPage === 1;
            prevBtn.onclick = () => {
                if (this.currentPage > 1) {
                    this.currentPage--;
                    this.render();
                }
            };
            nav.appendChild(prevBtn);

            for (let i = 1; i <= totalPages; i++) {
                if (i === 1 || i === totalPages || (i >= this.currentPage - 1 && i <= this.currentPage + 1)) {
                    const pageBtn = document.createElement('button');
                    pageBtn.className = `page-btn ${i === this.currentPage ? 'active' : ''}`;
                    pageBtn.type = 'button';
                    pageBtn.textContent = i;
                    pageBtn.onclick = () => {
                        this.currentPage = i;
                        this.render();
                    };
                    nav.appendChild(pageBtn);
                } else if (i === this.currentPage - 2 || i === this.currentPage + 2) {
                    const dots = document.createElement('span');
                    dots.style.cssText = 'color: #52525b; padding: 0 0.25rem; font-size: 0.75rem;';
                    dots.textContent = '...';
                    nav.appendChild(dots);
                }
            }

            const nextBtn = document.createElement('button');
            nextBtn.className = 'page-btn';
            nextBtn.type = 'button';
            nextBtn.innerHTML = '›';
            nextBtn.disabled = this.currentPage === totalPages || totalPages === 0;
            nextBtn.onclick = () => {
                if (this.currentPage < totalPages) {
                    this.currentPage++;
                    this.render();
                }
            };
            nav.appendChild(nextBtn);
        }

        exportCSV() {
            let csv = [];
            const headers = Array.from(this.table.querySelectorAll('thead th'))
                .map(th => `"${th.innerText.replace('⇅','').replace('▲','').replace('▼','').trim()}"`);
            csv.push(headers.join(','));

            this.filteredRows.forEach(row => {
                const rowData = row.data.map(val => `"${val.replace(/"/g, '""')}"`);
                csv.push(rowData.join(','));
            });

            const blob = new Blob(["\ufeff" + csv.join('\n')], { type: 'text/csv;charset=utf-8;' });
            this.downloadFile(blob, `${this.options.filename}.csv`);
        }

        exportExcel() {
            let html = '<html xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:x="urn:schemas-microsoft-com:office:excel"><head><meta charset="UTF-8"></head><body><table>';
            html += '<thead><tr>';
            this.table.querySelectorAll('thead th').forEach(th => {
                html += `<th>${th.innerText.replace('⇅','').replace('▲','').replace('▼','').trim()}</th>`;
            });
            html += '</tr></thead><tbody>';

            this.filteredRows.forEach(row => {
                html += '<tr>';
                row.data.forEach(val => html += `<td>${val}</td>`);
                html += '</tr>';
            });

            html += '</tbody></table></body></html>';

            const blob = new Blob([html], { type: 'application/vnd.ms-excel;charset=utf-8;' });
            this.downloadFile(blob, `${this.options.filename}.xls`);
        }

        exportPDF() {
            window.print();
        }

        downloadFile(blob, fileName) {
            const link = document.createElement('a');
            link.href = URL.createObjectURL(blob);
            link.download = fileName;
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
        }
    }

    function init(tableId, options = {}) {
        const table = typeof tableId === 'string' ? document.getElementById(tableId) : tableId;
        if (!table) return null;

        const engine = new DataTableEngine(table, options);
        instances.set(table.id || table, engine);
        return engine;
    }

    // Auto-inicialización declarativa
    function autoInit(container = document) {
        const tables = container.querySelectorAll('table[data-core-table], table.table');
        tables.forEach(table => {
            if (!table.dataset.coreTableInitialized) {
                table.dataset.coreTableInitialized = "true";
                const pageSize = parseInt(table.getAttribute('data-page-size'), 10) || 5;
                const filename = table.getAttribute('data-filename') || 'reporte_exportacion';
                init(table, { pageSize, filename });
            }
        });
    }

    return {
        init: init,
        autoInit: autoInit,
        getInstance: (id) => instances.get(id)
    };
})();
