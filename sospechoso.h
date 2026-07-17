#ifndef SOSPECHOSO_H
#define SOSPECHOSO_H

#include <string>
#include <vector>

// Límite práctico: el backtracking explora hasta 2^n nodos en el peor caso,
// así que este tope evita que alguien meta 40 sospechosos y el programa
// se cuelgue calculando 2^40 combinaciones en la demo en vivo.
const int MAX_SOSPECHOSOS = 24;

// Una declaración: el testigo (idTestigo) habla sobre otra persona (idAcusado).
// Es una arista DIRIGIDA cuando acusa, y se trata como arista del grafo de
// coartadas (no dirigida) cuando esCoartada = true.
struct Testimonio {
    int idTestigo;
    int idAcusado;
    std::string declaracion;
    bool esCoartada;   // true = defiende / confirma coartada | false = acusa
};

// Cada persona involucrada en el caso (sospechoso y a la vez testigo).
struct Sospechoso {
    int id;
    std::string nombre;
    double credibilidad;          // 0.0 - 1.0, usada por el módulo de Monte Carlo
    std::vector<int> coartadas;   // lista de adyacencia del grafo de coartadas

    Sospechoso() = default;
    Sospechoso(int _id, std::string _nombre, double _credibilidad)
        : id(_id), nombre(std::move(_nombre)), credibilidad(_credibilidad) {}
};

#endif
