#ifndef GRAFO_COARTADAS_H
#define GRAFO_COARTADAS_H

#include "caso.h"
#include <iostream>
#include <vector>
#include <set>

// ============================================================================
// GRAFO DE COARTADAS
// Estructura de datos: grafo NO dirigido, representado con lista de
// adyacencia (vector<vector<int>>). Cada nodo es un sospechoso; cada arista
// significa "estas dos personas se confirman la coartada mutuamente".
//
// Por qué importa para el caso: si el backtracking llega a marcar como
// culpables a dos personas conectadas por una arista, hay una contradicción
// lógica (un culpable no puede validar la coartada de otro culpable). El
// DFS de ciclos, además, delata **anillos de coartadas cerrados** (A avala a
// B, B avala a C, C avala a A...) que son sospechosos por sí mismos: sugieren
// un pacto de silencio entre 3 o más personas.
//
// Complejidad (V = sospechosos, E = testimonios tipo coartada):
//   construirListaAdyacencia   O(V + E)
//   dfsCiclos / DFS genérico   O(V + E) en los tres casos (mejor, promedio y
//                              peor) porque un DFS siempre visita cada nodo
//                              y cada arista como máximo una vez, sin importar
//                              la forma del grafo.
//   componentesConexas         O(V + E), mismo argumento, es un DFS/BFS
//                              repetido desde cada nodo no visitado.
// ============================================================================

using ListaAdyacencia = std::vector<std::vector<int>>;

// Construye la lista de adyacencia a partir de las coartadas del caso.
// O(V) para reservar + O(E) para copiar cada relación ya guardada en Sospechoso.
inline ListaAdyacencia construirListaAdyacencia(const Caso& caso) {
    ListaAdyacencia adj(caso.n());
    for (const auto& s : caso.getSospechosos()) {
        for (int vecino : s.coartadas) adj[s.id].push_back(vecino);
    }
    return adj;
}

inline void imprimirGrafo(const Caso& caso, const ListaAdyacencia& adj) {
    std::cout << "\n=== RED DE COARTADAS (grafo no dirigido, lista de adyacencia) ===\n";
    const auto& sospechosos = caso.getSospechosos();
    for (int i = 0; i < (int)adj.size(); i++) {
        std::cout << "  " << sospechosos[i].nombre << " -- coartada con --> ";
        if (adj[i].empty()) {
            std::cout << "(nadie)";
        } else {
            std::set<int> vistos; // por si hay coartadas duplicadas en el mismo par
            bool primero = true;
            for (int v : adj[i]) {
                if (vistos.count(v)) continue;
                vistos.insert(v);
                if (!primero) std::cout << ", ";
                std::cout << sospechosos[v].nombre;
                primero = false;
            }
        }
        std::cout << "\n";
    }
}

// DFS recursivo que detecta un ciclo en un grafo NO dirigido: se apoya en
// "padre" para no confundir la arista de vuelta al padre con un ciclo real.
// Si visita a un vecino ya visitado que NO es su padre inmediato, encontró
// un ciclo (un anillo de coartadas cerrado).
inline bool dfsCicloUtil(int u, int padre, const ListaAdyacencia& adj,
                          std::vector<bool>& visitado, std::vector<int>& cicloEncontrado) {
    visitado[u] = true;
    for (int v : adj[u]) {
        if (!visitado[v]) {
            if (dfsCicloUtil(v, u, adj, visitado, cicloEncontrado)) {
                if (cicloEncontrado.empty() || cicloEncontrado.back() != u)
                    cicloEncontrado.push_back(u);
                return true;
            }
        } else if (v != padre) {
            // Arista de retroceso: cierra un ciclo entre u y v
            cicloEncontrado.push_back(v);
            cicloEncontrado.push_back(u);
            return true;
        }
    }
    return false;
}

// Recorre todas las componentes (el grafo puede estar desconectado) y
// reporta si existe al menos un ciclo en la red completa.
inline bool dfsCiclos(const Caso& caso, const ListaAdyacencia& adj) {
    std::cout << "\n=== DETECCION DE CICLOS EN LA RED DE COARTADAS (DFS) ===\n";
    int n = (int)adj.size();
    std::vector<bool> visitado(n, false);
    bool hayCiclo = false;

    for (int i = 0; i < n; i++) {
        if (visitado[i]) continue;
        std::vector<int> ciclo;
        if (dfsCicloUtil(i, -1, adj, visitado, ciclo)) {
            hayCiclo = true;
            std::cout << "  [!] Anillo de coartadas detectado: ";
            const auto& sospechosos = caso.getSospechosos();
            for (size_t k = 0; k < ciclo.size(); k++) {
                std::cout << sospechosos[ciclo[k]].nombre;
                if (k + 1 < ciclo.size()) std::cout << " -> ";
            }
            std::cout << "  (posible pacto de silencio)\n";
        }
    }
    if (!hayCiclo) std::cout << "  No se encontraron ciclos: la red de coartadas es un bosque.\n";
    return hayCiclo;
}

// DFS auxiliar para marcar toda una componente conexa con la misma etiqueta.
inline void dfsMarcarComponente(int u, const ListaAdyacencia& adj,
                                 std::vector<int>& componenteDe, int etiqueta) {
    componenteDe[u] = etiqueta;
    for (int v : adj[u]) {
        if (componenteDe[v] == -1) dfsMarcarComponente(v, adj, componenteDe, etiqueta);
    }
}

// Agrupa a los sospechosos en "bloques" de coartadas cruzadas: gente sin
// ninguna conexión entre sí queda en componentes de tamaño 1 (sin coartada),
// mientras que grupos que se avalan mutuamente aparecen en el mismo bloque.
inline int componentesConexas(const Caso& caso, const ListaAdyacencia& adj) {
    std::cout << "\n=== COMPONENTES CONEXAS DE LA RED DE COARTADAS ===\n";
    int n = (int)adj.size();
    std::vector<int> componenteDe(n, -1);
    int totalComponentes = 0;

    for (int i = 0; i < n; i++) {
        if (componenteDe[i] == -1) {
            dfsMarcarComponente(i, adj, componenteDe, totalComponentes);
            totalComponentes++;
        }
    }

    const auto& sospechosos = caso.getSospechosos();
    for (int c = 0; c < totalComponentes; c++) {
        std::cout << "  Bloque " << c << ": ";
        bool primero = true;
        for (int i = 0; i < n; i++) {
            if (componenteDe[i] == c) {
                if (!primero) std::cout << ", ";
                std::cout << sospechosos[i].nombre;
                primero = false;
            }
        }
        std::cout << "\n";
    }
    std::cout << "  Total de bloques: " << totalComponentes << "\n";
    return totalComponentes;
}

#endif
