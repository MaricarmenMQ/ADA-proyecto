#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "caso.h"
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>

// ============================================================================
// SIMULACION MONTE CARLO — algoritmo probabilístico
// Cada testigo dice la verdad con probabilidad = su credibilidad. En cada
// una de las S simulaciones se "lanza una moneda cargada" por testimonio
// para decidir si ese testigo mintió o no en ese escenario, se acumula un
// puntaje de sospecha por acusado, y se cuenta en cuántos escenarios cada
// sospechoso terminó comprometido. Al final, frecuencia relativa = estimador
// de la probabilidad real de culpabilidad (Ley de los Grandes Números).
//
// ANÁLISIS DE COMPLEJIDAD (S = simulaciones, m = testimonios, n = sospechosos)
//   Los tres casos (mejor, promedio y peor) son IDÉNTICOS: es un algoritmo
//   de tiempo determinístico en su estructura de control (no hay poda ni
//   ramas que cortar), siempre ejecuta exactamente S * m operaciones para
//   acumular puntajes más S * n para tabular resultados.
//   Peor caso = Caso promedio = Mejor caso = O(S * (m + n))
//   (Lo único que varía entre corridas es el RESULTADO numérico, nunca el
//   número de operaciones — es la diferencia clave frente al backtracking,
//   donde la poda sí depende de los datos de entrada.)
// ============================================================================
inline std::vector<double> simulacionCulpabilidad(const Caso& caso, int simulaciones = 10000) {
    std::cout << "\n=== SIMULACION MONTE CARLO (" << simulaciones << " escenarios) ===\n";
    int n = caso.n();
    const auto& sospechosos = caso.getSospechosos();
    const auto& testimonios = caso.getTestimonios();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dado(0.0, 1.0);

    std::vector<long long> vecesComprometido(n, 0);

    for (int it = 0; it < simulaciones; it++) {
        std::vector<double> puntaje(n, 0.0);
        for (const auto& t : testimonios) {
            double azar = dado(gen);
            bool diceVerdad = (azar <= sospechosos[t.idTestigo].credibilidad);
            double efecto = t.esCoartada ? -0.5 : 1.0;
            if (!diceVerdad) efecto = -efecto;
            puntaje[t.idAcusado] += efecto;
        }
        for (int i = 0; i < n; i++) {
            if (puntaje[i] >= 0.99) vecesComprometido[i]++;
        }
    }

    std::vector<double> probabilidad(n, 0.0);
    for (int i = 0; i < n; i++) {
        probabilidad[i] = 100.0 * (double)vecesComprometido[i] / simulaciones;
        std::cout << "  " << std::left << std::setw(16) << sospechosos[i].nombre
                   << std::fixed << std::setprecision(2) << probabilidad[i] << " %\n";
    }
    return probabilidad;
}

#endif
