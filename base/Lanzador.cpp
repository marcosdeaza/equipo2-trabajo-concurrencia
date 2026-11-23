#include "Lanzador.h"

#include "Config.h"
#include "Estadisticas.h"
#include "Traza.h"

#include <algorithm>
#include <iostream>

Lanzador::Lanzador(std::string nombrePrograma, std::vector<std::string> opciones, std::vector<std::string> fallos)
    : _nombre(std::move(nombrePrograma)), _opciones(std::move(opciones)), _fallos(std::move(fallos)) {
    for (const char* comun : {"fallo", "estres", "traza", "vigilante", "ayuda"}) {
        _opciones.push_back(comun);
    }
}

void Lanzador::ayuda() {
    std::cout << "Uso: " << _nombre << " [--opcion valor]...\nOpciones:";
    for (const auto& o : _opciones) {
        std::cout << " --" << o;
    }
    std::cout << "\nModos de fallo (--fallo <nombre>):";
    for (const auto& f : _fallos) {
        std::cout << ' ' << f;
    }
    std::cout << "\nLos valores por defecto están en main.cpp.\n";
}

int Lanzador::ejecutar(const Config& config, FuncionSimulacion simulacion) {
    if (config.bandera("ayuda")) {
        ayuda();
        return 0;
    }
    auto desconocidas = config.clavesDesconocidas(_opciones);
    if (!desconocidas.empty()) {
        std::cout << "Opción desconocida: --" << desconocidas.front() << '\n';
        ayuda();
        return 2;
    }
    std::string fallo = config.texto("fallo", "");
    if (!fallo.empty() && std::find(_fallos.begin(), _fallos.end(), fallo) == _fallos.end()) {
        std::cout << "Modo de fallo desconocido: " << fallo << '\n';
        ayuda();
        return 2;
    }
    std::cout << _nombre << (fallo.empty() ? " (versión correcta)" : " --fallo " + fallo) << '\n';

    int repeticiones = config.entero("estres", 0);
    if (repeticiones <= 0) {
        Estadisticas estadisticas;
        Traza traza(config.bandera("traza"));
        bool ok = simulacion(config, estadisticas, traza, "ejecución única");
        estadisticas.imprimir(std::cout);
        return ok ? 0 : 1;
    }

    int fallidas = 0;
    for (int k = 1; k <= repeticiones; ++k) {
        Estadisticas estadisticas;
        Traza traza(false);
        std::string etiqueta = "ejecución " + std::to_string(k) + " de " + std::to_string(repeticiones);
        bool ok = simulacion(config, estadisticas, traza, etiqueta);
        if (!ok) {
            ++fallidas;
            std::cout << "  " << etiqueta << ": INVARIANTE ROTO -> " << estadisticas.violaciones().front() << '\n';
        }
    }
    std::cout << "\n--estres " << repeticiones << ": " << (repeticiones - fallidas) << " OK, "
              << fallidas << " con invariantes rotos (misma configuración en todas).\n";
    return fallidas == 0 ? 0 : 1;
}
