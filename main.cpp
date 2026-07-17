#include "caso.h"
#include "grafo_coartadas.h"
#include "backtracking.h"
#include "montecarlo.h"
#include "ordenamiento.h"

#include <iostream>
#include <limits>
#include <string>

void pausar() {
    std::cout << "\nPresiona ENTER para continuar...";
    std::cin.get();
}

void limpiarBufer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void mostrarMenu(const Caso& caso) {
    std::cout << "\n=============================================\n";
    std::cout << "   DETECTIVE ALGORITMICO — " << caso.getNombre() << "\n";
    std::cout << "   Grafos + Backtracking (CSP) + Monte Carlo + QuickSort\n";
    std::cout << "=============================================\n";
    std::cout << " --- Gestion del caso (dinamico, no estatico) ---\n";
    std::cout << " 1. Ver sospechosos y testimonios cargados\n";
    std::cout << " 2. Agregar un sospechoso\n";
    std::cout << " 3. Agregar un testimonio\n";
    std::cout << " 4. Nuevo caso en blanco (armar tu propia historia)\n";
    std::cout << " 5. Cargar un caso predefinido de ejemplo (hay 3)\n";
    std::cout << " 6. Cargar caso desde archivo .txt\n";
    std::cout << " 7. Guardar caso actual en archivo .txt\n";
    std::cout << " --- Algoritmos ---\n";
    std::cout << " 8. Construir y mostrar grafo de coartadas\n";
    std::cout << " 9. Detectar ciclos en el grafo (DFS)\n";
    std::cout << "10. Mostrar componentes conexas (DFS)\n";
    std::cout << "11. Resolver combinacion de culpables (Backtracking + poda)\n";
    std::cout << "12. Simular culpabilidad (Monte Carlo)\n";
    std::cout << "13. Mostrar ranking final (QuickSort)\n";
    std::cout << "14. Ver analisis de complejidad consolidado\n";
    std::cout << "15. Ejecutar TODO en secuencia (modo expo)\n";
    std::cout << " 0. Salir\n";
    std::cout << "Opcion: ";
}

void nuevoCasoInteractivo(Caso& caso) {
    std::string nombre;
    std::cout << "Titulo de tu caso (Ej: El robo de la biblioteca): ";
    std::getline(std::cin, nombre);
    caso.nuevoCasoEnBlanco(nombre);
    std::cout << "Caso en blanco creado: \"" << caso.getNombre() << "\".\n";
    std::cout << "Ahora usa la opcion 2 para agregar sospechosos y la opcion 3\n";
    std::cout << "para agregar testimonios; nada viene precargado.\n";
}

void elegirCasoPredefinido(Caso& caso) {
    std::cout << "\nCasos de ejemplo disponibles:\n";
    std::cout << "  1. El Enigma del Vagon Azul       (6 sospechosos)\n";
    std::cout << "  2. El asesinato de Bonifacio Choquehuanca (7 sospechosos)\n";
    std::cout << "  3. El cargamento desaparecido del puerto  (4 sospechosos)\n";
    std::cout << "Elige uno (1-3): ";
    int variante;
    std::cin >> variante;
    limpiarBufer();
    if (variante < 1 || variante > 3) variante = 1;
    caso.cargarCasoPredefinido(variante);
    std::cout << "Caso cargado: \"" << caso.getNombre() << "\".\n";
}

void agregarSospechosoInteractivo(Caso& caso) {
    std::string nombre;
    double credibilidad;
    std::cout << "Nombre del sospechoso: ";
    std::getline(std::cin, nombre);
    std::cout << "Credibilidad (0.0 a 1.0): ";
    std::cin >> credibilidad;
    limpiarBufer();
    int id = caso.agregarSospechoso(nombre, credibilidad);
    if (id == -1) {
        std::cout << "No se pudo agregar: se llego al maximo de " << MAX_SOSPECHOSOS << " sospechosos.\n";
    } else {
        std::cout << nombre << " agregado con id " << id << ".\n";
    }
}

void agregarTestimonioInteractivo(Caso& caso) {
    if (caso.n() < 2) {
        std::cout << "Necesitas al menos 2 sospechosos cargados antes de agregar un testimonio.\n";
        return;
    }
    caso.listar();
    int idTestigo, idAcusado, esCoartada;
    std::string declaracion;
    std::cout << "Id del testigo: ";
    std::cin >> idTestigo;
    std::cout << "Id del acusado: ";
    std::cin >> idAcusado;
    std::cout << "Es coartada? (1 = si defiende, 0 = si acusa): ";
    std::cin >> esCoartada;
    limpiarBufer();
    std::cout << "Declaracion textual: ";
    std::getline(std::cin, declaracion);

    bool ok = caso.agregarTestimonio(idTestigo, idAcusado, esCoartada == 1, declaracion);
    std::cout << (ok ? "Testimonio agregado.\n" : "IDs invalidos, no se agrego el testimonio.\n");
}

void mostrarAnalisisComplejidad(const Caso& caso, long long nodosExplorados) {
    std::cout << "\n=== ANALISIS DE COMPLEJIDAD CONSOLIDADO ===\n";
    int n = caso.n();
    long long espacio = 1LL << n;

    std::cout << "\n[Grafo de coartadas — DFS de ciclos / componentes conexas]\n";
    std::cout << "  Peor caso = Caso promedio = Mejor caso : O(V + E)\n";
    std::cout << "  (V=" << n << " sospechosos, E=testimonios de coartada). Un DFS\n";
    std::cout << "  visita cada nodo y cada arista como maximo una vez, sin\n";
    std::cout << "  importar la forma del grafo.\n";

    std::cout << "\n[Backtracking — CSP con bitmask + poda]\n";
    std::cout << "  Peor caso     : O(2^n * m)      -- casi sin contradicciones,\n";
    std::cout << "                  se explora casi todo el arbol binario.\n";
    std::cout << "  Caso promedio : O(2^(n/2) * m)  [empirico] -- la poda por\n";
    std::cout << "                  consistencia y por cota corta ramas completas.\n";
    std::cout << "  Mejor caso    : O(n * m)        -- la primera rama probada ya\n";
    std::cout << "                  es consistente con todos los testimonios.\n";
    std::cout << "  En esta ejecucion se exploraron " << nodosExplorados
               << " de " << espacio << " nodos posibles.\n";

    std::cout << "\n[Monte Carlo — algoritmo probabilistico]\n";
    std::cout << "  Peor caso = Caso promedio = Mejor caso : O(S * (n + m))\n";
    std::cout << "  No hay poda ni ramas: siempre se ejecutan las S simulaciones\n";
    std::cout << "  completas, solo cambia el resultado numerico, no el trabajo.\n";

    std::cout << "\n[QuickSort — ranking final por probabilidad]\n";
    std::cout << "  Peor caso     : O(n^2)      -- entrada ya (casi) ordenada con\n";
    std::cout << "                  pivote fijo en el ultimo elemento.\n";
    std::cout << "  Caso promedio : O(n log n)  -- particiones razonablemente\n";
    std::cout << "                  balanceadas con datos en orden aleatorio.\n";
    std::cout << "  Mejor caso    : O(n log n)  -- el pivote parte el arreglo\n";
    std::cout << "                  exactamente a la mitad en cada llamada.\n";
}

int main(int argc, char** argv) {
    Caso caso;
    caso.cargarCasoPredefinido();

    bool modoAutomatico = (argc > 1 && std::string(argv[1]) == "--auto");

    if (modoAutomatico) {
        auto adj = construirListaAdyacencia(caso);
        imprimirGrafo(caso, adj);
        dfsCiclos(caso, adj);
        componentesConexas(caso, adj);

        Backtracking bt(caso);
        bt.resolverCaso();

        auto prob = simulacionCulpabilidad(caso, 10000);
        mostrarRankingFinal(caso, prob, bt.getMejorMascara(), bt.getHuboSolucion());
        mostrarAnalisisComplejidad(caso, bt.getNodosExplorados());
        return 0;
    }

    int opcion = -1;
    do {
        mostrarMenu(caso);
        std::cin >> opcion;
        limpiarBufer();

        static long long ultimosNodosExplorados = 0;
        static std::vector<double> ultimaProbabilidad;
        static int ultimaMascara = 0;
        static bool ultimaHuboSolucion = false;

        switch (opcion) {
            case 1: caso.listar(); pausar(); break;
            case 2: agregarSospechosoInteractivo(caso); pausar(); break;
            case 3: agregarTestimonioInteractivo(caso); pausar(); break;
            case 4: nuevoCasoInteractivo(caso); pausar(); break;
            case 5: elegirCasoPredefinido(caso); pausar(); break;
            case 6: {
                std::string ruta;
                std::cout << "Ruta del archivo .txt: ";
                std::getline(std::cin, ruta);
                if (!caso.cargarDesdeArchivo(ruta)) std::cout << "No se pudo abrir el archivo.\n";
                else std::cout << "Caso cargado desde " << ruta << ".\n";
                pausar();
                break;
            }
            case 7: {
                std::string ruta;
                std::cout << "Ruta de salida .txt: ";
                std::getline(std::cin, ruta);
                caso.guardarEnArchivo(ruta);
                std::cout << "Caso guardado en " << ruta << ".\n";
                pausar();
                break;
            }
            case 8: {
                auto adj = construirListaAdyacencia(caso);
                imprimirGrafo(caso, adj);
                pausar();
                break;
            }
            case 9: {
                auto adj = construirListaAdyacencia(caso);
                dfsCiclos(caso, adj);
                pausar();
                break;
            }
            case 10: {
                auto adj = construirListaAdyacencia(caso);
                componentesConexas(caso, adj);
                pausar();
                break;
            }
            case 11: {
                Backtracking bt(caso);
                bt.resolverCaso();
                ultimosNodosExplorados = bt.getNodosExplorados();
                ultimaMascara = bt.getMejorMascara();
                ultimaHuboSolucion = bt.getHuboSolucion();
                pausar();
                break;
            }
            case 12:
                ultimaProbabilidad = simulacionCulpabilidad(caso, 10000);
                pausar();
                break;
            case 13:
                if (ultimaProbabilidad.empty()) {
                    std::cout << "Primero corre la opcion 12 (Monte Carlo).\n";
                } else {
                    mostrarRankingFinal(caso, ultimaProbabilidad, ultimaMascara, ultimaHuboSolucion);
                }
                pausar();
                break;
            case 14:
                mostrarAnalisisComplejidad(caso, ultimosNodosExplorados);
                pausar();
                break;
            case 15: {
                auto adj = construirListaAdyacencia(caso);
                imprimirGrafo(caso, adj);
                dfsCiclos(caso, adj);
                componentesConexas(caso, adj);

                Backtracking bt(caso);
                bt.resolverCaso();
                ultimosNodosExplorados = bt.getNodosExplorados();
                ultimaMascara = bt.getMejorMascara();
                ultimaHuboSolucion = bt.getHuboSolucion();

                ultimaProbabilidad = simulacionCulpabilidad(caso, 10000);
                mostrarRankingFinal(caso, ultimaProbabilidad, ultimaMascara, ultimaHuboSolucion);
                mostrarAnalisisComplejidad(caso, ultimosNodosExplorados);
                pausar();
                break;
            }
            case 0: std::cout << "Cerrando el caso...\n"; break;
            default: std::cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);

    return 0;
}
