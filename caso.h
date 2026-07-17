#ifndef CASO_H
#define CASO_H

#include "sospechoso.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// Caso: contiene los DATOS (sospechosos + testimonios) y nada de algoritmos.
// A diferencia de la primera versión, el caso YA NO es estático: se puede
// arrancar vacío y construirlo sospechoso por sospechoso / testimonio por
// testimonio en tiempo de ejecución, además de tener un caso predefinido
// de ejemplo y poder cargar/guardar en archivo de texto.
// ============================================================================
class Caso {
private:
    std::vector<Sospechoso> sospechosos;
    std::vector<Testimonio> testimonios;
    std::string nombreCaso;

public:
    Caso() : nombreCaso("Caso sin titulo") {}

    // -------- Construcción dinámica --------
    // Devuelve el id asignado, o -1 si ya se llegó a MAX_SOSPECHOSOS.
    int agregarSospechoso(const std::string& nombre, double credibilidad) {
        if ((int)sospechosos.size() >= MAX_SOSPECHOSOS) return -1;
        int id = (int)sospechosos.size();
        sospechosos.emplace_back(id, nombre, credibilidad);
        return id;
    }

    // Devuelve false si algún id no existe (evita testimonios "fantasma").
    bool agregarTestimonio(int idTestigo, int idAcusado, bool esCoartada, const std::string& declaracion) {
        if (idTestigo < 0 || idTestigo >= (int)sospechosos.size()) return false;
        if (idAcusado < 0 || idAcusado >= (int)sospechosos.size()) return false;
        if (idTestigo == idAcusado) return false;

        testimonios.push_back({idTestigo, idAcusado, declaracion, esCoartada});
        if (esCoartada) {
            // La coartada es una relación no dirigida en el grafo: si A dice
            // "yo estaba con B", ambos quedan conectados en la red.
            sospechosos[idTestigo].coartadas.push_back(idAcusado);
            sospechosos[idAcusado].coartadas.push_back(idTestigo);
        }
        return true;
    }

    void limpiar() {
        sospechosos.clear();
        testimonios.clear();
        nombreCaso = "Caso sin titulo";
    }

    void setNombre(const std::string& n) { nombreCaso = n; }

    // -------- Caso de ejemplo (solo para tener algo con qué arrancar) --------
    // Hay 3 variantes distintas para elegir desde el menu, las mismas dos que
    // trae la interfaz web (Hacienda / Puerto) mas la original (Vagon Azul),
    // para que la consola tenga la misma variedad de demos que la web.
    void cargarCasoPredefinido(int variante = 1) {
        limpiar();
        switch (variante) {
            case 2: cargarCasoHacienda(); break;
            case 3: cargarCasoPuerto();   break;
            default: cargarCasoVagonAzul(); break;
        }
    }

    // -------- Caso nuevo completamente en blanco --------
    // A diferencia de cargarCasoPredefinido(), esto NO trae ningun sospechoso
    // ni testimonio: es el punto de partida para armar una historia propia
    // desde cero usando agregarSospechoso()/agregarTestimonio() (opciones 2 y 3
    // del menu de consola).
    void nuevoCasoEnBlanco(const std::string& nombre) {
        limpiar();
        nombreCaso = nombre.empty() ? "Nuevo caso sin titulo" : nombre;
    }

private:
    void cargarCasoVagonAzul() {
        nombreCaso = "El Enigma del Vagon Azul";

        agregarSospechoso("Ama de Llaves", 0.85);
        agregarSospechoso("Mayordomo",     0.90);
        agregarSospechoso("Chef",          0.70);
        agregarSospechoso("Jardinero",     0.60);
        agregarSospechoso("Hija",          0.95);
        agregarSospechoso("Chofer",        0.75);

        agregarTestimonio(0, 1, false, "El mayordomo estaba en el jardin");
        agregarTestimonio(1, 0, false, "El ama de llaves estaba en su cuarto");
        agregarTestimonio(2, 4, false, "La hija estaba discutiendo con el");
        agregarTestimonio(3, 2, true,  "El chef estaba cocinando, lo vi");
        agregarTestimonio(4, 5, false, "El chofer llego tarde esa noche");
        agregarTestimonio(5, 3, true,  "El jardinero estaba en la puerta");
        // Coartada extra SOLO entre inocentes (Chef-Jardinero-Chofer), para que
        // el grafo tenga un ciclo real que mostrar en la demo del DFS, sin
        // contradecir la solucion del backtracking (Mayordomo e Hija culpables).
        agregarTestimonio(2, 5, true,  "El chef tambien vio al chofer en la cocina");
    }

    // Mismo caso que "El asesinato de Bonifacio Choquehuanca" de la interfaz
    // web (mismos sospechosos, mismos testimonios), para que consola y web
    // muestren exactamente el mismo resultado si se elige esta variante.
    void cargarCasoHacienda() {
        nombreCaso = "El asesinato de Bonifacio Choquehuanca";

        agregarSospechoso("Anacleto", 0.80); // Mayordomo
        agregarSospechoso("Herminia", 0.72); // Cocinera
        agregarSospechoso("Reynaldo", 0.55); // Sobrino y heredero
        agregarSospechoso("Flora",    0.88); // Ama de llaves
        agregarSospechoso("Aurelio",  0.65); // Jardinero
        agregarSospechoso("Deyxi",    0.70); // Empleada
        agregarSospechoso("Wilfredo", 0.60); // Chofer

        agregarTestimonio(4, 2, false, "Aurelio vio a Reynaldo entrar solo a las 20:50");
        agregarTestimonio(0, 3, true,  "Anacleto y Flora estuvieron juntos ordenando el estudio");
        agregarTestimonio(3, 0, true,  "Flora confirma que Anacleto no salio del comedor");
        agregarTestimonio(1, 6, true,  "Herminia vio a Wilfredo en la cocina toda la noche");
        agregarTestimonio(6, 1, true,  "Wilfredo confirma haber estado con Herminia");
        agregarTestimonio(5, 2, false, "Deyxi escucho una discusion de Reynaldo con el hacendado");
        agregarTestimonio(2, 4, false, "Reynaldo acusa a Aurelio de rondar el estudio");
    }

    // Mismo caso que "El cargamento desaparecido del puerto" de la web.
    void cargarCasoPuerto() {
        nombreCaso = "El cargamento desaparecido del puerto";

        agregarSospechoso("Marco",  0.68); // Vigilante nocturno
        agregarSospechoso("Elsa",   0.77); // Operadora de grua
        agregarSospechoso("Tito",   0.58); // Supervisor de turno
        agregarSospechoso("Rocio",  0.83); // Administradora

        agregarTestimonio(1, 0, false, "Elsa vio a Marco alejarse de su puesto sin autorizacion");
        agregarTestimonio(3, 2, false, "Rocio encontro registros de acceso alterados por Tito");
        agregarTestimonio(0, 1, true,  "Marco confirma que Elsa opero la grua sin interrupciones");
    }

public:

    // -------- Carga desde archivo de texto plano --------
    // Formato por linea:
    //   PERSONA;nombre;credibilidad
    //   TESTIMONIO;idTestigo;idAcusado;esCoartada(0/1);declaracion
    bool cargarDesdeArchivo(const std::string& ruta) {
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;

        limpiar();
        nombreCaso = "Caso cargado desde " + ruta;

        std::string linea;
        while (std::getline(archivo, linea)) {
            if (linea.empty() || linea[0] == '#') continue;
            std::stringstream ss(linea);
            std::string token;
            std::vector<std::string> campos;
            while (std::getline(ss, token, ';')) campos.push_back(token);
            if (campos.empty()) continue;

            if (campos[0] == "PERSONA" && campos.size() >= 3) {
                agregarSospechoso(campos[1], std::stod(campos[2]));
            } else if (campos[0] == "TESTIMONIO" && campos.size() >= 5) {
                agregarTestimonio(std::stoi(campos[1]), std::stoi(campos[2]),
                                   campos[3] == "1", campos[4]);
            }
        }
        return true;
    }

    // -------- Guardado a archivo (para persistir un caso armado a mano) --------
    bool guardarEnArchivo(const std::string& ruta) const {
        std::ofstream archivo(ruta);
        if (!archivo.is_open()) return false;
        archivo << "# " << nombreCaso << "\n";
        for (const auto& s : sospechosos)
            archivo << "PERSONA;" << s.nombre << ";" << s.credibilidad << "\n";
        for (const auto& t : testimonios)
            archivo << "TESTIMONIO;" << t.idTestigo << ";" << t.idAcusado << ";"
                     << (t.esCoartada ? 1 : 0) << ";" << t.declaracion << "\n";
        return true;
    }

    // -------- Consultas --------
    int n() const { return (int)sospechosos.size(); }
    const std::vector<Sospechoso>& getSospechosos() const { return sospechosos; }
    const std::vector<Testimonio>& getTestimonios() const { return testimonios; }
    const std::string& getNombre() const { return nombreCaso; }

    void listar() const {
        std::cout << "\n--- Caso: " << nombreCaso << " ---\n";
        std::cout << "Sospechosos (" << sospechosos.size() << "):\n";
        for (const auto& s : sospechosos)
            std::cout << "  [" << s.id << "] " << s.nombre
                       << "  (credibilidad " << s.credibilidad * 100 << "%)\n";

        std::cout << "Testimonios (" << testimonios.size() << "):\n";
        for (const auto& t : testimonios) {
            std::cout << "  " << sospechosos[t.idTestigo].nombre
                       << (t.esCoartada ? " confirma coartada de " : " acusa a ")
                       << sospechosos[t.idAcusado].nombre
                       << "  -> \"" << t.declaracion << "\"\n";
        }
    }
};

#endif
