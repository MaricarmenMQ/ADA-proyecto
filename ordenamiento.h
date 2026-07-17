#ifndef ORDENAMIENTO_H
#define ORDENAMIENTO_H

#include "caso.h"
#include <vector>
#include <utility>
#include <iostream>
#include <iomanip>

// ============================================================================
// QUICKSORT — arma el ranking final ordenando a los sospechosos por su
// probabilidad de culpabilidad (salida de Monte Carlo), de mayor a menor.
// Partición estilo Lomuto, pivote = último elemento.
//
// ANÁLISIS DE COMPLEJIDAD (n = numero de sospechosos a ordenar)
//   Peor caso     O(n^2)      — ocurre cuando la lista ya viene ordenada (o
//                               casi) y se usa siempre el último elemento
//                               como pivote: cada partición solo separa un
//                               elemento del resto, degenerando en n niveles
//                               de recursion en vez de log n.
//   Caso promedio O(n log n)  — con probabilidades en orden aleatorio, cada
//                               partición divide el arreglo en dos mitades
//                               razonablemente parejas, dando log n niveles
//                               de profundidad con n comparaciones cada uno.
//   Mejor caso    O(n log n)  — el pivote parte el arreglo exactamente a la
//                               mitad en cada llamada.
//   (n en este proyecto es el numero de sospechosos, tipicamente pequeño;
//   se eligio quicksort sobre todo por su valor didactico de divide y
//   venceras, no por ser estrictamente necesario a esta escala.)
// ============================================================================

using Ranking = std::vector<std::pair<int, double>>; // (idSospechoso, probabilidad)

inline int particion(Ranking& v, int bajo, int alto) {
    double pivote = v[alto].second;
    int i = bajo - 1;
    for (int j = bajo; j < alto; j++) {
        if (v[j].second > pivote) { // orden DESCENDENTE: mas sospechoso primero
            i++;
            std::swap(v[i], v[j]);
        }
    }
    std::swap(v[i + 1], v[alto]);
    return i + 1;
}

inline void quicksortRanking(Ranking& v, int bajo, int alto) {
    if (bajo < alto) {
        int p = particion(v, bajo, alto);
        quicksortRanking(v, bajo, p - 1);
        quicksortRanking(v, p + 1, alto);
    }
}

inline void mostrarRankingFinal(const Caso& caso, const std::vector<double>& probabilidad,
                                 int mejorMascara, bool huboSolucionCSP) {
    std::cout << "\n=== RANKING FINAL (ordenado por QuickSort) ===\n";
    int n = caso.n();
    Ranking ranking;
    for (int i = 0; i < n; i++) ranking.push_back({i, probabilidad[i]});
    quicksortRanking(ranking, 0, (int)ranking.size() - 1);

    const auto& sospechosos = caso.getSospechosos();
    for (const auto& [id, prob] : ranking) {
        bool culpableCSP = huboSolucionCSP && ((mejorMascara >> id) & 1);
        std::cout << "  " << std::left << std::setw(16) << sospechosos[id].nombre
                   << std::fixed << std::setprecision(1) << std::setw(6) << prob << "%  "
                   << "CSP: " << (culpableCSP ? "CULPABLE" : "inocente") << "\n";
    }
}

#endif
