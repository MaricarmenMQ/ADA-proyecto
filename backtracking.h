#ifndef BACKTRACKING_H
#define BACKTRACKING_H

#include "caso.h"
#include <iostream>
#include <iomanip>

// ============================================================================
// BACKTRACKING (CSP: Constraint Satisfaction Problem)
// Cada sospechoso es una variable booleana (culpable / inocente). El espacio
// de búsqueda completo son las 2^n asignaciones posibles, representadas aquí
// con un ENTERO usado como bitmask (bit i = 1 significa "sospechoso i es
// culpable"). Se recorre ese árbol binario probando inocente/culpable en
// cada nivel, y se PODA (branch & bound) toda rama que:
//   a) ya usa tantos o más culpables que la mejor solución encontrada, o
//   b) contradice un testimonio ya decidido (forward checking).
//
// Resultado buscado: la asignación consistente con MENOS culpables (la
// hipótesis más simple que no contradice ningún testimonio).
//
// ANÁLISIS DE COMPLEJIDAD (n = número de sospechosos, m = testimonios)
//   Peor caso    O(2^n * m)  — los testimonios casi no se contradicen entre
//                              sí, así que la poda por consistencia apenas
//                              actúa y se recorre casi todo el árbol binario;
//                              en cada uno de los ~2^n nodos se revisan los
//                              m testimonios para chequear consistencia.
//   Caso promedio  O(2^(n/2) * m) [empírico] — con testimonios cruzados
//                              típicos, la poda por consistencia y la cota
//                              por número de culpables descartan ramas
//                              enteras muy temprano; en la práctica el árbol
//                              explorado se reduce a una fracción exponencial
//                              menor, no lineal.
//   Mejor caso   O(n * m)     — la primera rama probada (todos inocentes)
//                              ya es consistente con todos los testimonios;
//                              el algoritmo termina tras un solo camino de
//                              longitud n, revisando m testimonios por nivel.
// ============================================================================
class Backtracking {
private:
    const Caso& caso;
    int n;
    int mejorMascara;
    int mejorNumCulpables;
    long long nodosExplorados;
    bool huboSolucion;

    // Revisa consistencia SOLO de las variables ya decididas (índices < limite).
    // Evaluar variables aún no asignadas rompería el forward checking y podaría
    // ramas válidas por error.
    bool esConsistente(int mascara, int limite) const {
        for (const auto& t : caso.getTestimonios()) {
            if (t.idTestigo >= limite || t.idAcusado >= limite) continue;
            bool testigoCulpable = (mascara >> t.idTestigo) & 1;
            bool acusadoCulpable = (mascara >> t.idAcusado) & 1;
            if (!testigoCulpable) { // testigo inocente => su declaracion es verdadera
                if (t.esCoartada) {
                    if (acusadoCulpable) return false; // avala a alguien que resulto culpable
                } else {
                    if (!acusadoCulpable) return false; // acusa a alguien que resulto inocente
                }
            }
        }
        // Dos personas con coartada mutua no pueden ser culpables ambas
        for (const auto& s : caso.getSospechosos()) {
            if (s.id >= limite) continue;
            bool sCulpable = (mascara >> s.id) & 1;
            if (!sCulpable) continue;
            for (int c : s.coartadas) {
                if (c >= limite) continue;
                bool cCulpable = (mascara >> c) & 1;
                if (cCulpable) return false;
            }
        }
        return true;
    }

    void resolver(int indice, int mascaraActual, int culpablesActuales) {
        nodosExplorados++;
        if (culpablesActuales >= mejorNumCulpables) return; // poda por cota

        if (indice == n) {
            if (esConsistente(mascaraActual, n)) {
                mejorNumCulpables = culpablesActuales;
                mejorMascara = mascaraActual;
                huboSolucion = true;
            }
            return;
        }

        // Rama 1: sospechoso `indice` es inocente (bit en 0, ya lo está)
        if (esConsistente(mascaraActual, indice + 1))
            resolver(indice + 1, mascaraActual, culpablesActuales);

        // Rama 2: sospechoso `indice` es culpable (se prende el bit)
        int conCulpable = mascaraActual | (1 << indice);
        if (esConsistente(conCulpable, indice + 1))
            resolver(indice + 1, conCulpable, culpablesActuales + 1);
    }

public:
    explicit Backtracking(const Caso& c)
        : caso(c), n(c.n()), mejorMascara(0),
          mejorNumCulpables(c.n() + 1), nodosExplorados(0), huboSolucion(false) {}

    void resolverCaso() {
        mejorMascara = 0;
        mejorNumCulpables = n + 1;
        nodosExplorados = 0;
        huboSolucion = false;

        std::cout << "\n=== BACKTRACKING (CSP) — bitmask + poda ===\n";
        resolver(0, 0, 0);

        if (!huboSolucion) {
            std::cout << "No existe asignacion consistente con los testimonios.\n";
            return;
        }
        const auto& sospechosos = caso.getSospechosos();
        for (const auto& s : sospechosos) {
            bool culpable = (mejorMascara >> s.id) & 1;
            std::cout << "  " << std::left << std::setw(16) << s.nombre
                       << (culpable ? "-> CULPABLE" : "-> inocente") << "\n";
        }
        long long espacioTotal = 1LL << n;
        std::cout << "Numero minimo de culpables consistente: " << mejorNumCulpables << "\n";
        std::cout << "Nodos explorados (con poda): " << nodosExplorados
                   << " de " << espacioTotal << " posibles ("
                   << std::fixed << std::setprecision(1)
                   << (100.0 - 100.0 * nodosExplorados / (2.0 * espacioTotal)) << "% podado)\n";
    }

    long long getNodosExplorados() const { return nodosExplorados; }
    int getMejorMascara() const { return mejorMascara; }
    int getMejorNumCulpables() const { return mejorNumCulpables; }
    bool getHuboSolucion() const { return huboSolucion; }
};

#endif
