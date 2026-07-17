// ============================================================================
// Detective Algorítmico — motor de expedientes dinámicos
// La lógica de grafo (DFS), backtracking (CSP + poda), Monte Carlo y
// QuickSort es la misma que en src/*.h; lo único nuevo aquí es que ahora
// puede haber VARIOS expedientes en memoria y se cambia de uno a otro sin
// perder nada. Ningún sospechoso ni testimonio está "quemado" en el código:
// todo sale de los expedientes de abajo, que el usuario puede editar.
// ============================================================================

function nuevoCasoVacio(titulo) {
  return {
    id: 'DA-' + String(Date.now()).slice(-3),
    titulo: titulo || 'Nuevo expediente',
    ventana: '—',
    objeto: '—',
    descripcion: '',
    sospechosos: [],
    testimonios: []
  };
}

function casoDemoHacienda() {
  const c = nuevoCasoVacio('El asesinato de Bonifacio Choquehuanca');
  c.ventana = '20:30 — 21:00';
  c.objeto = 'Bonifacio Choquehuanca';
  c.descripcion = 'Hacienda Wiñay Marka, a orillas del lago Titicaca. El hacendado fue encontrado sin vida en su estudio.';
  c.sospechosos = [
    { id: 0, nombre: 'Anacleto', rol: 'Mayordomo', credibilidad: 0.80, coartadas: [] },
    { id: 1, nombre: 'Herminia', rol: 'Cocinera', credibilidad: 0.72, coartadas: [] },
    { id: 2, nombre: 'Reynaldo', rol: 'Sobrino y heredero', credibilidad: 0.55, coartadas: [] },
    { id: 3, nombre: 'Flora', rol: 'Ama de llaves', credibilidad: 0.88, coartadas: [] },
    { id: 4, nombre: 'Aurelio', rol: 'Jardinero', credibilidad: 0.65, coartadas: [] },
    { id: 5, nombre: 'Deyxi', rol: 'Empleada', credibilidad: 0.70, coartadas: [] },
    { id: 6, nombre: 'Wilfredo', rol: 'Chofer', credibilidad: 0.60, coartadas: [] }
  ];
  const t = [
    [4, 2, false, 'Aurelio vio a Reynaldo entrar solo a las 20:50'],
    [0, 3, true,  'Anacleto y Flora estuvieron juntos ordenando el estudio'],
    [3, 0, true,  'Flora confirma que Anacleto no salió del comedor'],
    [1, 6, true,  'Herminia vio a Wilfredo en la cocina toda la noche'],
    [6, 1, true,  'Wilfredo confirma haber estado con Herminia'],
    [5, 2, false, 'Deyxi escuchó una discusión de Reynaldo con el hacendado'],
    [2, 4, false, 'Reynaldo acusa a Aurelio de rondar el estudio']
  ];
  t.forEach(([a, b, coartada, decl]) => {
    c.testimonios.push({ idTestigo: a, idAcusado: b, esCoartada: coartada, declaracion: decl });
    if (coartada) {
      c.sospechosos[a].coartadas.push(b);
      c.sospechosos[b].coartadas.push(a);
    }
  });
  return c;
}

function casoDemoPuerto() {
  const c = nuevoCasoVacio('El cargamento desaparecido del puerto');
  c.ventana = '02:00 — 03:30';
  c.objeto = 'Contenedor #A-114';
  c.descripcion = 'Terminal portuario de Ilo. Un contenedor con mercadería declarada desapareció durante el turno nocturno.';
  c.sospechosos = [
    { id: 0, nombre: 'Marco', rol: 'Vigilante nocturno', credibilidad: 0.68, coartadas: [] },
    { id: 1, nombre: 'Elsa', rol: 'Operadora de grúa', credibilidad: 0.77, coartadas: [] },
    { id: 2, nombre: 'Tito', rol: 'Supervisor de turno', credibilidad: 0.58, coartadas: [] },
    { id: 3, nombre: 'Rocío', rol: 'Administradora', credibilidad: 0.83, coartadas: [] }
  ];
  const t = [
    [1, 0, false, 'Elsa vio a Marco alejarse de su puesto sin autorización'],
    [3, 2, false, 'Rocío encontró registros de acceso alterados por Tito'],
    [0, 1, true,  'Marco confirma que Elsa operó la grúa sin interrupciones']
  ];
  t.forEach(([a, b, coartada, decl]) => {
    c.testimonios.push({ idTestigo: a, idAcusado: b, esCoartada: coartada, declaracion: decl });
    if (coartada) {
      c.sospechosos[a].coartadas.push(b);
      c.sospechosos[b].coartadas.push(a);
    }
  });
  return c;
}

let casos = [casoDemoHacienda(), casoDemoPuerto()];
let casoIdx = 0;
function caso() { return casos[casoIdx]; }

let ultimaMascara = 0;
let ultimoHuboSolucion = false;
let ultimaProbabilidad = [];
let ultimosCiclosDetectados = null;

// ============================================================================
// CONSOLA — log en vivo
// ============================================================================
function logTerm(tag, texto) {
  const term = document.getElementById('terminal');
  const hora = new Date().toLocaleTimeString('es-PE', { hour12: false });
  const div = document.createElement('div');
  div.className = 'log-line';
  div.innerHTML = `<span class="log-time">${hora}</span><span class="log-tag tag-${tag}">[${tag}]</span>${texto}`;
  term.appendChild(div);
  term.scrollTop = term.scrollHeight;
}
function limpiarLog() { document.getElementById('terminal').innerHTML = ''; }
const delay = (ms) => new Promise((r) => setTimeout(r, ms));

// ============================================================================
// PIPELINE lateral
// ============================================================================
function marcarPipeline(paso, done) {
  const li = document.querySelector(`.pipeline li[data-step="${paso}"]`);
  if (!li) return;
  li.classList.toggle('done', !!done);
}
function activarPipeline(paso) {
  document.querySelectorAll('.pipeline li').forEach((li) => li.classList.remove('active'));
  const li = document.querySelector(`.pipeline li[data-step="${paso}"]`);
  if (li) li.classList.add('active');
}
function reiniciarPipeline() {
  document.querySelectorAll('.pipeline li').forEach((li) => li.classList.remove('active', 'done'));
}

// ============================================================================
// GESTIÓN DE EXPEDIENTES (alta/baja de sospechosos y testimonios)
// ============================================================================
function agregarSospechoso(nombre, rol, credibilidad) {
  const s = caso().sospechosos;
  const id = s.length;
  s.push({ id, nombre, rol: rol || '—', credibilidad, coartadas: [] });
  return id;
}

function agregarTestimonio(idTestigo, idAcusado, esCoartada, declaracion) {
  const s = caso().sospechosos;
  if (idTestigo === idAcusado) return false;
  if (!s[idTestigo] || !s[idAcusado]) return false;
  caso().testimonios.push({ idTestigo, idAcusado, esCoartada, declaracion });
  if (esCoartada) {
    s[idTestigo].coartadas.push(idAcusado);
    s[idAcusado].coartadas.push(idTestigo);
  }
  return true;
}

function eliminarSospechoso(id) {
  const c = caso();
  const restantes = c.sospechosos.filter((s) => s.id !== id).map((s) => ({ ...s }));
  const mapaNuevoId = new Map();
  restantes.forEach((s, i) => mapaNuevoId.set(s.id, i));

  const nuevosTestimonios = c.testimonios
    .filter((t) => t.idTestigo !== id && t.idAcusado !== id)
    .map((t) => ({
      idTestigo: mapaNuevoId.get(t.idTestigo),
      idAcusado: mapaNuevoId.get(t.idAcusado),
      esCoartada: t.esCoartada,
      declaracion: t.declaracion
    }));

  c.sospechosos = restantes.map((s, i) => ({ id: i, nombre: s.nombre, rol: s.rol, credibilidad: s.credibilidad, coartadas: [] }));
  c.testimonios = [];
  nuevosTestimonios.forEach((t) => agregarTestimonio(t.idTestigo, t.idAcusado, t.esCoartada, t.declaracion));
}

function eliminarTestimonio(index) {
  const c = caso();
  const t = c.testimonios[index];
  if (t.esCoartada) {
    const a = c.sospechosos[t.idTestigo].coartadas;
    a.splice(a.indexOf(t.idAcusado), 1);
    const b = c.sospechosos[t.idAcusado].coartadas;
    b.splice(b.indexOf(t.idTestigo), 1);
  }
  c.testimonios.splice(index, 1);
}

// ============================================================================
// RENDER — sincroniza el DOM con el expediente activo
// ============================================================================
function iniciales(nombre) {
  return nombre.trim().split(/\s+/).slice(0, 2).map((p) => p[0].toUpperCase()).join('');
}

// Emoji representativo según el rol del sospechoso — puramente visual,
// no afecta ningún cálculo del motor (grafo / CSP / Monte Carlo).
function emojiPorRol(rol) {
  const r = (rol || '').toLowerCase();
  const mapa = [
    [/mayordomo|butler/, '🎩'],
    [/cocin/, '👩\u200d🍳'],
    [/sobrin|hered/, '🧑'],
    [/ama de llaves|limpi/, '🧹'],
    [/jardin/, '🌿'],
    [/emplead/, '🧑\u200d💼'],
    [/chofer|conductor/, '🚗'],
    [/vigilant|guardia|seguridad/, '🛡️'],
    [/gr[uú]a|operad/, '🏗️'],
    [/supervisor|turno/, '📋'],
    [/administrad|contad/, '🗂️'],
    [/m[eé]dic|doctor/, '🩺'],
    [/abogad/, '⚖️'],
    [/socio|negocio|empresari/, '💼'],
    [/polic[ií]a|detective/, '🕵️'],
  ];
  for (const [re, e] of mapa) if (re.test(r)) return e;
  return '👤';
}

function renderSelectorCasos() {
  const sel = document.getElementById('selCaso');
  sel.innerHTML = '';
  casos.forEach((c, i) => {
    sel.innerHTML += `<option value="${i}">${String(i + 1).padStart(2, '0')} — ${c.titulo}</option>`;
  });
  sel.value = casoIdx;
}

function renderCabecera() {
  const c = caso();
  document.getElementById('caseIdTag').textContent = `EXPEDIENTE #${c.id}`;
  document.getElementById('caseTitulo').value = c.titulo;
  document.getElementById('caseDescripcion').value = c.descripcion;
  document.getElementById('metaVentana').value = c.ventana;
  document.getElementById('metaObjeto').value = c.objeto;
}

function renderTodo() {
  renderSelectorCasos();
  renderCabecera();
  renderTablaSospechosos();
  renderSelects();
  renderTablaTestimonios();
  drawBoard(null);
  resetResultados();
  actualizarStatsBase();
  reiniciarPipeline();
  marcarPipeline('cargar', caso().sospechosos.length > 0);
}

function renderTablaSospechosos() {
  const tbody = document.querySelector('#tablaSospechosos tbody');
  tbody.innerHTML = '';
  caso().sospechosos.forEach((s) => {
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${s.id}</td><td>${s.nombre}</td><td>${s.rol || '—'}</td><td>${Math.round(s.credibilidad * 100)}%</td>
      <td class="row-del" data-id="${s.id}">eliminar</td>`;
    tr.querySelector('.row-del').addEventListener('click', () => { eliminarSospechoso(s.id); renderTodo(); });
    tbody.appendChild(tr);
  });
}

function renderSelects() {
  const selTestigo = document.getElementById('inTestigo');
  const selAcusado = document.getElementById('inAcusado');
  selTestigo.innerHTML = '';
  selAcusado.innerHTML = '';
  caso().sospechosos.forEach((s) => {
    selTestigo.innerHTML += `<option value="${s.id}">${s.nombre}</option>`;
    selAcusado.innerHTML += `<option value="${s.id}">${s.nombre}</option>`;
  });
}

function renderTablaTestimonios() {
  const tbody = document.querySelector('#tablaTestimonios tbody');
  tbody.innerHTML = '';
  const s = caso().sospechosos;
  caso().testimonios.forEach((t, i) => {
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${s[t.idTestigo].nombre}</td>
      <td style="color:var(--text-faint);font-family:var(--font-mono);font-size:10.5px;">${t.esCoartada ? 'confirma coartada de' : 'acusa a'}</td>
      <td>${s[t.idAcusado].nombre}</td>
      <td style="color:var(--text-dim);">"${t.declaracion}"</td>
      <td class="row-del" data-i="${i}">eliminar</td>`;
    tr.querySelector('.row-del').addEventListener('click', () => { eliminarTestimonio(i); renderTodo(); });
    tbody.appendChild(tr);
  });
}

function resetResultados() {
  document.getElementById('tablaRanking').style.display = 'none';
  document.getElementById('barras').innerHTML = '';
  document.getElementById('btnReset').disabled = true;
  document.getElementById('btnMonteCarlo').disabled = true;
  document.getElementById('btnRanking').disabled = true;
  document.getElementById('btnResolver').disabled = caso().sospechosos.length === 0;
  document.getElementById('badgeCiclos').textContent = '— ciclos';
  document.getElementById('badgeCSP').textContent = 'pendiente';
  document.getElementById('badgeMC').textContent = '0 completadas';
  document.getElementById('cspNombre').textContent = '—';
  document.getElementById('cspIcon').textContent = '🕵️';
  document.getElementById('cspNodos').textContent = '—';
  document.getElementById('cspCulpables').textContent = '—';
  document.getElementById('cspAhorro').textContent = '—';
  document.getElementById('statHipotesis').textContent = '—';
  document.getElementById('statHipotesisSub').textContent = 'sin ejecutar aún';
  document.getElementById('statPodas').textContent = '—';
  actualizarDeclaracionDestacada();
  ultimaMascara = 0; ultimoHuboSolucion = false; ultimaProbabilidad = []; ultimosCiclosDetectados = null;
}

function actualizarDeclaracionDestacada(nombreFoco) {
  const box = document.getElementById('declaracionTexto');
  const s = caso().sospechosos;
  const t = caso().testimonios;
  if (!t.length) { box.textContent = 'Aún no hay testimonios cargados en este expediente.'; return; }
  let elegido = null;
  if (nombreFoco) elegido = t.find((x) => !x.esCoartada && s[x.idAcusado].nombre === nombreFoco);
  if (!elegido) elegido = t.find((x) => !x.esCoartada) || t[0];
  box.textContent = `"${elegido.declaracion}" — declaración de ${s[elegido.idTestigo].nombre}.`;
}

function actualizarStatsBase() {
  const n = caso().sospechosos.length;
  const aristas = new Set();
  caso().sospechosos.forEach((s) => s.coartadas.forEach((c) => aristas.add([Math.min(s.id, c), Math.max(s.id, c)].join('-'))));
  document.getElementById('statSujetos').textContent = n;
  document.getElementById('statCoartadas').textContent = aristas.size;
  document.getElementById('statN').textContent = n;
  document.getElementById('statEspacio').textContent = n > 0 ? (1 << n) : 1;
  document.getElementById('statNodos').textContent = '—';
  document.getElementById('statAhorro').textContent = '—';
}

// ============================================================================
// GRAFO — dibujo SVG + DFS de ciclos + componentes conexas
// ============================================================================
const svgNS = 'http://www.w3.org/2000/svg';

function posiciones() {
  const n = caso().sospechosos.length;
  const cx = 360, cy = 215, r = 150;
  const pos = [];
  for (let i = 0; i < n; i++) {
    const ang = (-Math.PI / 2) + i * (2 * Math.PI / Math.max(n, 1));
    pos.push({ x: cx + r * Math.cos(ang), y: cy + r * Math.sin(ang) });
  }
  return pos;
}

function drawBoard(mascaraCulpables) {
  const board = document.getElementById('board');
  board.innerHTML = '';
  const s = caso().sospechosos;
  const pos = posiciones();

  const defs = document.createElementNS(svgNS, 'defs');
  defs.innerHTML = `<marker id="arrow" markerWidth="8" markerHeight="8" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 z" fill="#e0a638" opacity="0.75"/></marker>`;
  board.appendChild(defs);

  const enCiclo = new Set();
  if (ultimosCiclosDetectados) ultimosCiclosDetectados.forEach((ciclo) => {
    for (let i = 0; i < ciclo.length - 1; i++) {
      enCiclo.add([Math.min(ciclo[i], ciclo[i + 1]), Math.max(ciclo[i], ciclo[i + 1])].join('-'));
    }
  });

  const dibujadas = new Set();
  s.forEach((sus) => {
    sus.coartadas.forEach((c) => {
      const key = [Math.min(sus.id, c), Math.max(sus.id, c)].join('-');
      if (dibujadas.has(key)) return;
      dibujadas.add(key);
      const a = pos[sus.id], b = pos[c];
      const line = document.createElementNS(svgNS, 'line');
      line.setAttribute('x1', a.x); line.setAttribute('y1', a.y);
      line.setAttribute('x2', b.x); line.setAttribute('y2', b.y);
      line.setAttribute('class', 'edge-line');
      let color = '#a9b8d1', width = '1.4', dash = '2 4';
      if (mascaraCulpables !== null && ((mascaraCulpables >> sus.id) & 1) && ((mascaraCulpables >> c) & 1)) {
        color = '#e0463a'; width = '2'; dash = 'none';
      } else if (enCiclo.has(key)) {
        color = '#12a877'; width = '2.5'; dash = 'none';
      }
      line.setAttribute('stroke', color);
      line.setAttribute('stroke-width', width);
      if (dash !== 'none') line.setAttribute('stroke-dasharray', dash);
      board.appendChild(line);
    });
  });

  caso().testimonios.forEach((t) => {
    if (t.esCoartada) return;
    const a = pos[t.idTestigo], b = pos[t.idAcusado];
    const mx = (a.x + b.x) / 2 + (a.y - b.y) * 0.08, my = (a.y + b.y) / 2 + (b.x - a.x) * 0.08;
    const path = document.createElementNS(svgNS, 'path');
    path.setAttribute('d', `M ${a.x} ${a.y} Q ${mx} ${my} ${b.x} ${b.y}`);
    path.setAttribute('stroke', '#e0a638');
    path.setAttribute('stroke-width', '1.3');
    path.setAttribute('stroke-dasharray', '5 5');
    path.setAttribute('class', 'edge-accuse');
    path.setAttribute('fill', 'none');
    path.setAttribute('opacity', '0.6');
    path.setAttribute('marker-end', 'url(#arrow)');
    board.appendChild(path);
  });

  s.forEach((sus, i) => {
    const p = pos[i];
    const culpable = mascaraCulpables !== null && ((mascaraCulpables >> sus.id) & 1);
    const g = document.createElementNS(svgNS, 'g');
    g.setAttribute('class', 'node-group');
    g.style.animationDelay = (i * 0.07) + 's';

    // halo pulsante detrás del nodo culpable
    if (culpable) {
      const pulse1 = document.createElementNS(svgNS, 'circle');
      pulse1.setAttribute('cx', p.x); pulse1.setAttribute('cy', p.y); pulse1.setAttribute('r', 26);
      pulse1.setAttribute('fill', 'none'); pulse1.setAttribute('stroke', '#e0463a'); pulse1.setAttribute('stroke-width', '2');
      pulse1.innerHTML = `<animate attributeName="r" values="26;42;26" dur="2.2s" repeatCount="indefinite"/>
        <animate attributeName="opacity" values="0.55;0;0.55" dur="2.2s" repeatCount="indefinite"/>`;
      g.appendChild(pulse1);
    }

    const circ = document.createElementNS(svgNS, 'circle');
    circ.setAttribute('cx', p.x); circ.setAttribute('cy', p.y); circ.setAttribute('r', 25);
    circ.setAttribute('class', 'node-circle');
    circ.setAttribute('fill', culpable ? '#fdecea' : '#ffffff');
    circ.setAttribute('stroke', culpable ? '#e0463a' : '#2f6fed');
    circ.setAttribute('stroke-width', culpable ? '2.4' : '1.8');
    if (mascaraCulpables === null) circ.setAttribute('stroke-dasharray', '3 3');
    g.appendChild(circ);

    // emoji del rol dentro del nodo
    const emo = document.createElementNS(svgNS, 'text');
    emo.setAttribute('x', p.x); emo.setAttribute('y', p.y + 7);
    emo.setAttribute('text-anchor', 'middle');
    emo.setAttribute('font-size', '20');
    emo.textContent = emojiPorRol(sus.rol);
    g.appendChild(emo);

    // insignia de culpable: cuchillo sobre el nodo
    if (culpable) {
      const badgeBg = document.createElementNS(svgNS, 'circle');
      badgeBg.setAttribute('cx', p.x + 18); badgeBg.setAttribute('cy', p.y - 18); badgeBg.setAttribute('r', 11);
      badgeBg.setAttribute('fill', '#e0463a');
      g.appendChild(badgeBg);
      const badge = document.createElementNS(svgNS, 'text');
      badge.setAttribute('x', p.x + 18); badge.setAttribute('y', p.y - 14);
      badge.setAttribute('text-anchor', 'middle');
      badge.setAttribute('font-size', '12');
      badge.textContent = '🔪';
      g.appendChild(badge);
    }

    const label = document.createElementNS(svgNS, 'text');
    label.setAttribute('x', p.x); label.setAttribute('y', p.y + 40);
    label.setAttribute('text-anchor', 'middle');
    label.setAttribute('font-size', '11.5');
    label.setAttribute('font-family', 'Inter, sans-serif');
    label.setAttribute('font-weight', '600');
    label.setAttribute('fill', '#152238');
    label.textContent = sus.nombre;
    g.appendChild(label);
    const rol = document.createElementNS(svgNS, 'text');
    rol.setAttribute('x', p.x); rol.setAttribute('y', p.y + 53);
    rol.setAttribute('text-anchor', 'middle');
    rol.setAttribute('font-size', '9.5');
    rol.setAttribute('font-family', 'Inter, sans-serif');
    rol.setAttribute('fill', '#5b6c88');
    rol.textContent = sus.rol || '';
    g.appendChild(rol);
    board.appendChild(g);
  });
}

function construirListaAdyacencia() {
  const s = caso().sospechosos;
  const adj = Array.from({ length: s.length }, () => []);
  s.forEach((sus) => sus.coartadas.forEach((c) => adj[sus.id].push(c)));
  return adj;
}

function dfsCiclos(silencioso) {
  const s = caso().sospechosos;
  const adj = construirListaAdyacencia();
  const n = adj.length;
  const visitado = new Array(n).fill(false);
  const ciclosEncontrados = [];

  function dfsUtil(u, padre, ciclo) {
    visitado[u] = true;
    for (const v of adj[u]) {
      if (!visitado[v]) {
        if (dfsUtil(v, u, ciclo)) {
          if (!ciclo.length || ciclo[ciclo.length - 1] !== u) ciclo.push(u);
          return true;
        }
      } else if (v !== padre) {
        ciclo.push(v); ciclo.push(u);
        return true;
      }
    }
    return false;
  }

  for (let i = 0; i < n; i++) {
    if (visitado[i]) continue;
    const ciclo = [];
    if (dfsUtil(i, -1, ciclo)) ciclosEncontrados.push(ciclo);
  }

  ultimosCiclosDetectados = ciclosEncontrados;
  document.getElementById('badgeCiclos').textContent = `${ciclosEncontrados.length} ciclo${ciclosEncontrados.length === 1 ? '' : 's'}`;

  if (!silencioso) {
    if (!ciclosEncontrados.length) {
      logTerm('GRAFO', 'DFS completado: no se hallaron ciclos, la red de coartadas es un bosque.');
    } else {
      ciclosEncontrados.forEach((c) => {
        logTerm('GRAFO', `Anillo de coartadas: ${c.map((id) => s[id].nombre).join(' → ')} (posible pacto de silencio).`);
      });
    }
  }
  drawBoard(ultimoHuboSolucion ? ultimaMascara : null);
  marcarPipeline('dfs', true);
}

function componentesConexas() {
  const s = caso().sospechosos;
  const adj = construirListaAdyacencia();
  const n = adj.length;
  const compDe = new Array(n).fill(-1);
  let total = 0;

  function marcar(u, etiqueta) {
    compDe[u] = etiqueta;
    for (const v of adj[u]) if (compDe[v] === -1) marcar(v, etiqueta);
  }
  for (let i = 0; i < n; i++) if (compDe[i] === -1) { marcar(i, total); total++; }

  for (let c = 0; c < total; c++) {
    const nombres = s.filter((_, i) => compDe[i] === c).map((sus) => sus.nombre);
    logTerm('GRAFO', `Bloque ${c}: ${nombres.join(', ')}.`);
  }
  logTerm('GRAFO', `Total de bloques conexos: ${total}.`);
}

// ============================================================================
// BACKTRACKING (CSP) — bitmask + poda
// ============================================================================
function esConsistente(mascara, limite) {
  const s = caso().sospechosos;
  for (const t of caso().testimonios) {
    if (t.idTestigo >= limite || t.idAcusado >= limite) continue;
    const testigoCulpable = (mascara >> t.idTestigo) & 1;
    const acusadoCulpable = (mascara >> t.idAcusado) & 1;
    if (!testigoCulpable) {
      if (t.esCoartada) { if (acusadoCulpable) return false; }
      else { if (!acusadoCulpable) return false; }
    }
  }
  for (const sus of s) {
    if (sus.id >= limite) continue;
    if (!((mascara >> sus.id) & 1)) continue;
    for (const c of sus.coartadas) {
      if (c >= limite) continue;
      if ((mascara >> c) & 1) return false;
    }
  }
  return true;
}

function ejecutarBacktracking() {
  const s = caso().sospechosos;
  const n = s.length;
  if (n === 0) return;
  let nodosExplorados = 0;
  let mejorMascara = 0;
  let mejorNumCulpables = n + 1;
  let huboSolucion = false;

  function resolver(indice, mascaraActual, culpablesActuales) {
    nodosExplorados++;
    if (culpablesActuales >= mejorNumCulpables) return;
    if (indice === n) {
      if (esConsistente(mascaraActual, n)) {
        mejorNumCulpables = culpablesActuales;
        mejorMascara = mascaraActual;
        huboSolucion = true;
      }
      return;
    }
    if (esConsistente(mascaraActual, indice + 1)) resolver(indice + 1, mascaraActual, culpablesActuales);
    const conCulpable = mascaraActual | (1 << indice);
    if (esConsistente(conCulpable, indice + 1)) resolver(indice + 1, conCulpable, culpablesActuales + 1);
  }
  resolver(0, 0, 0);

  ultimaMascara = mejorMascara;
  ultimoHuboSolucion = huboSolucion;

  const espacio = n > 0 ? (1 << n) : 1;
  const ahorro = Math.max(0, 100 - 100 * nodosExplorados / (2 * espacio));
  const podas = Math.max(0, 2 * espacio - nodosExplorados);

  document.getElementById('statNodos').textContent = nodosExplorados;
  document.getElementById('statAhorro').textContent = ahorro.toFixed(0) + '%';
  document.getElementById('statPodas').textContent = podas;
  document.getElementById('cspNodos').textContent = nodosExplorados;
  document.getElementById('cspAhorro').textContent = ahorro.toFixed(0) + '%';
  document.getElementById('badgeCSP').textContent = 'resuelto';

  if (!huboSolucion) {
    logTerm('CSP', `No existe asignación consistente con los testimonios registrados. Nodos explorados: ${nodosExplorados}/${espacio}.`);
    document.getElementById('cspNombre').textContent = 'Sin solución consistente';
    document.getElementById('cspCulpables').textContent = '0';
    document.getElementById('cspIcon').textContent = '🕵️';
  } else {
    const culpables = s.filter((sus) => (mejorMascara >> sus.id) & 1).map((sus) => sus.nombre);
    logTerm('CSP', `Solución encontrada con ${mejorNumCulpables} culpable(s). Poda del ${ahorro.toFixed(1)}% del espacio de búsqueda.`);
    document.getElementById('cspNombre').textContent = culpables.length ? culpables.join(', ') : 'Todos inocentes';
    document.getElementById('cspCulpables').textContent = mejorNumCulpables;
    document.getElementById('cspIcon').textContent = culpables.length ? '🔪' : '🕊️';
    actualizarDeclaracionDestacada(culpables[0]);
    if (culpables.length) document.getElementById('statHipotesis').textContent = culpables[0];
    if (culpables.length) { document.getElementById('statHipotesisSub').textContent = 'según backtracking CSP'; }
  }

  const tbody = document.querySelector('#tablaResultado tbody');
  if (tbody) {
    tbody.innerHTML = '';
    s.forEach((sus) => {
      const culpable = huboSolucion && ((mejorMascara >> sus.id) & 1);
      const tr = document.createElement('tr');
      tr.innerHTML = `<td>${sus.nombre}</td><td>${Math.round(sus.credibilidad * 100)}%</td>
        <td class="tag-veredicto ${culpable ? 'guilty' : 'innocent'}">${culpable ? 'Culpable' : 'Inocente'}</td>`;
      tbody.appendChild(tr);
    });
  }
  drawBoard(huboSolucion ? mejorMascara : null);

  document.getElementById('btnReset').disabled = false;
  document.getElementById('btnMonteCarlo').disabled = n === 0;
  document.getElementById('btnResolver').disabled = true;
  marcarPipeline('bt', true);
}

// ============================================================================
// MONTE CARLO
// ============================================================================
function ejecutarMonteCarlo() {
  const s = caso().sospechosos;
  const n = s.length;
  if (n === 0) return;
  const iteraciones = 10000;
  const contador = new Array(n).fill(0);

  for (let it = 0; it < iteraciones; it++) {
    const puntaje = new Array(n).fill(0);
    for (const t of caso().testimonios) {
      const azar = Math.random();
      const diceVerdad = azar <= s[t.idTestigo].credibilidad;
      let efecto = t.esCoartada ? -0.5 : 1.0;
      if (!diceVerdad) efecto = -efecto;
      puntaje[t.idAcusado] += efecto;
    }
    for (let i = 0; i < n; i++) if (puntaje[i] >= 0.99) contador[i]++;
  }

  ultimaProbabilidad = s.map((sus, i) => 100 * contador[i] / iteraciones);
  document.getElementById('badgeMC').textContent = `${iteraciones.toLocaleString('es-PE')} completadas`;

  const cont = document.getElementById('barras');
  cont.innerHTML = '';
  s.forEach((sus, i) => {
    const prob = ultimaProbabilidad[i];
    const row = document.createElement('div');
    row.className = 'bar-row';
    row.innerHTML = `<div class="bar-name">${sus.nombre}</div>
      <div class="bar-track"><div class="bar-fill" style="width:0%"></div></div>
      <div class="bar-val">${prob.toFixed(1)}%</div>`;
    cont.appendChild(row);
    requestAnimationFrame(() => { row.querySelector('.bar-fill').style.width = prob + '%'; });
  });

  const iMax = ultimaProbabilidad.reduce((best, v, i) => v > ultimaProbabilidad[best] ? i : best, 0);
  document.getElementById('statHipotesis').textContent = s[iMax].nombre;
  document.getElementById('statHipotesisSub').textContent = `${ultimaProbabilidad[iMax].toFixed(1)}% en Monte Carlo`;

  logTerm('MONTECARLO', `${iteraciones.toLocaleString('es-PE')} escenarios simulados. Mayor sospecha: ${s[iMax].nombre} (${ultimaProbabilidad[iMax].toFixed(1)}%).`);
  document.getElementById('btnRanking').disabled = false;
  marcarPipeline('mc', true);
}

// ============================================================================
// QUICKSORT
// ============================================================================
function particion(arr, bajo, alto) {
  const pivote = arr[alto].prob;
  let i = bajo - 1;
  for (let j = bajo; j < alto; j++) {
    if (arr[j].prob > pivote) { i++; [arr[i], arr[j]] = [arr[j], arr[i]]; }
  }
  [arr[i + 1], arr[alto]] = [arr[alto], arr[i + 1]];
  return i + 1;
}
function quicksortRanking(arr, bajo, alto) {
  if (bajo < alto) {
    const p = particion(arr, bajo, alto);
    quicksortRanking(arr, bajo, p - 1);
    quicksortRanking(arr, p + 1, alto);
  }
}

function ejecutarRanking() {
  const s = caso().sospechosos;
  const ranking = s.map((sus, i) => ({ id: sus.id, nombre: sus.nombre, prob: ultimaProbabilidad[i] || 0 }));
  quicksortRanking(ranking, 0, ranking.length - 1);

  const tbody = document.querySelector('#tablaRanking tbody');
  tbody.innerHTML = '';
  ranking.forEach((r, i) => {
    const culpable = ultimoHuboSolucion && ((ultimaMascara >> r.id) & 1);
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${i + 1}</td><td>${r.nombre}</td><td>${r.prob.toFixed(1)}%</td>
      <td class="tag-veredicto ${culpable ? 'guilty' : 'innocent'}">${culpable ? 'Culpable' : 'Inocente'}</td>`;
    tbody.appendChild(tr);
  });
  document.getElementById('tablaRanking').style.display = 'table';
  logTerm('RANKING', `Ranking final ordenado por QuickSort. #1: ${ranking[0] ? ranking[0].nombre : '—'}.`);
  marcarPipeline('qs', true);
}

// ============================================================================
// EJECUCIÓN COMPLETA (botón maestro de la barra lateral)
// ============================================================================
async function ejecutarInvestigacionCompleta() {
  const n = caso().sospechosos.length;
  if (n === 0) { logTerm('ERROR', 'No hay sospechosos cargados en este expediente.'); return; }
  document.getElementById('btnEjecutarTodo').disabled = true;
  resetResultados();
  logTerm('SISTEMA', `Expediente cargado: ${n} sospechosos, ${caso().testimonios.length} testimonios.`);
  marcarPipeline('cargar', true);

  mostrarVista('resumen');
  activarPipeline('grafo'); await delay(350);
  drawBoard(null);
  marcarPipeline('grafo', true);
  logTerm('GRAFO', 'Lista de adyacencia construida a partir de las coartadas cruzadas.');

  activarPipeline('dfs'); await delay(450);
  dfsCiclos();

  activarPipeline('bt'); await delay(500);
  ejecutarBacktracking();

  activarPipeline('mc'); await delay(600);
  mostrarVista('analisis');
  ejecutarMonteCarlo();

  activarPipeline('qs'); await delay(500);
  ejecutarRanking();

  activarPipeline('');
  logTerm('EXITO', 'Investigación completa. Revisa la hipótesis líder en Resumen y el ranking final aquí en Análisis.');
  document.getElementById('btnEjecutarTodo').disabled = false;
}

// ============================================================================
// NAVEGACIÓN POR PANTALLAS (tabs)
// ============================================================================
function mostrarVista(nombre) {
  document.querySelectorAll('.tab').forEach((b) => b.classList.toggle('active', b.dataset.view === nombre));
  document.querySelectorAll('.view').forEach((v) => v.classList.toggle('active', v.id === 'view-' + nombre));
}
document.querySelectorAll('.tab').forEach((btn) => {
  btn.addEventListener('click', () => mostrarVista(btn.dataset.view));
});

// ============================================================================
// EVENTOS
// ============================================================================
document.getElementById('selCaso').addEventListener('change', (e) => {
  casoIdx = parseInt(e.target.value);
  limpiarLog();
  logTerm('SISTEMA', `Expediente cambiado a "${caso().titulo}".`);
  renderTodo();
});

document.getElementById('btnNuevoCaso').addEventListener('click', () => {
  casos.push(nuevoCasoVacio());
  casoIdx = casos.length - 1;
  limpiarLog();
  logTerm('SISTEMA', 'Nuevo expediente creado. Ve a "Sospechosos & testimonios" para redactar tu historia.');
  renderTodo();
  mostrarVista('resumen');
  document.getElementById('caseTitulo').focus();
  document.getElementById('caseTitulo').select();
});

document.getElementById('caseTitulo').addEventListener('input', (e) => {
  caso().titulo = e.target.value;
  renderSelectorCasos();
});
document.getElementById('caseDescripcion').addEventListener('input', (e) => { caso().descripcion = e.target.value; });
document.getElementById('metaVentana').addEventListener('input', (e) => { caso().ventana = e.target.value; });
document.getElementById('metaObjeto').addEventListener('input', (e) => { caso().objeto = e.target.value; });

document.getElementById('btnEjecutarTodo').addEventListener('click', ejecutarInvestigacionCompleta);
document.getElementById('btnReiniciar').addEventListener('click', () => {
  limpiarLog();
  logTerm('SISTEMA', 'Análisis reiniciado.');
  renderTodo();
});
document.getElementById('btnLimpiarLog').addEventListener('click', limpiarLog);

document.getElementById('btnDFSCiclos').addEventListener('click', () => { activarPipeline('dfs'); dfsCiclos(); });
document.getElementById('btnComponentes').addEventListener('click', componentesConexas);

document.getElementById('btnResolver').addEventListener('click', () => { activarPipeline('bt'); ejecutarBacktracking(); });
document.getElementById('btnReset').addEventListener('click', () => { resetResultados(); drawBoard(null); });

document.getElementById('btnMonteCarlo').addEventListener('click', () => { activarPipeline('mc'); ejecutarMonteCarlo(); });
document.getElementById('btnRanking').addEventListener('click', () => { activarPipeline('qs'); ejecutarRanking(); });

document.getElementById('btnAddSospechoso').addEventListener('click', () => {
  const nombre = document.getElementById('inNombre').value.trim();
  const rol = document.getElementById('inRol').value.trim();
  const cred = parseFloat(document.getElementById('inCred').value);
  if (!nombre) return;
  if (caso().sospechosos.length >= 24) { alert('Máximo 24 sospechosos (2^24 sería demasiado para una demo en vivo).'); return; }
  agregarSospechoso(nombre, rol, isNaN(cred) ? 0.5 : Math.min(1, Math.max(0, cred)));
  document.getElementById('inNombre').value = '';
  document.getElementById('inRol').value = '';
  logTerm('SISTEMA', `Sospechoso agregado: ${nombre}${rol ? ' (' + rol + ')' : ''}.`);
  renderTodo();
});

document.getElementById('btnAddTestimonio').addEventListener('click', () => {
  const idTestigo = parseInt(document.getElementById('inTestigo').value);
  const idAcusado = parseInt(document.getElementById('inAcusado').value);
  const esCoartada = document.getElementById('inTipo').value === 'coartada';
  const declaracion = document.getElementById('inDeclaracion').value.trim() || '(sin declaración)';
  if (isNaN(idTestigo) || isNaN(idAcusado)) return;
  const ok = agregarTestimonio(idTestigo, idAcusado, esCoartada, declaracion);
  if (!ok) { alert('El testigo y el acusado deben ser personas distintas.'); return; }
  document.getElementById('inDeclaracion').value = '';
  logTerm('SISTEMA', 'Testimonio agregado al expediente.');
  renderTodo();
});

document.getElementById('btnVaciar').addEventListener('click', () => {
  caso().sospechosos = [];
  caso().testimonios = [];
  logTerm('SISTEMA', 'Datos del expediente vaciados.');
  renderTodo();
});

document.querySelectorAll('.pipeline li').forEach((li) => {
  li.addEventListener('click', () => {
    const paso = li.dataset.step;
    activarPipeline(paso);
    if (paso === 'cargar' || paso === 'grafo' || paso === 'dfs' || paso === 'bt') mostrarVista('resumen');
    if (paso === 'mc' || paso === 'qs') mostrarVista('analisis');
    if (paso === 'dfs') dfsCiclos();
    else if (paso === 'bt') ejecutarBacktracking();
    else if (paso === 'mc' && !document.getElementById('btnMonteCarlo').disabled) ejecutarMonteCarlo();
    else if (paso === 'qs' && !document.getElementById('btnRanking').disabled) ejecutarRanking();
    else if (paso === 'grafo') drawBoard(ultimoHuboSolucion ? ultimaMascara : null);
  });
});

// -------------------- arranque --------------------
logTerm('SISTEMA', 'Sistema de investigación inicializado.');
renderTodo();
