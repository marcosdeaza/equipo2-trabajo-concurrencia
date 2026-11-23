// Programa de la competición. NO SE MODIFICA.
//   ./competicion --hilos 4               una medición
//   ./competicion --hilos 4 --veces 5     5 mediciones y su mediana
//   ./competicion --comprobar             calcula también el resultado correcto (tarda)

#include "Producto.h"
#include "Valor.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    int hilos = 1;
    int veces = 1;
    bool comprobar = false;
    for (int a = 1; a < argc; ++a) {
        std::string arg = argv[a];
        if (arg == "--hilos" && a + 1 < argc) {
            hilos = std::atoi(argv[++a]);
        } else if (arg == "--veces" && a + 1 < argc) {
            veces = std::atoi(argv[++a]);
        } else if (arg == "--comprobar") {
            comprobar = true;
        } else {
            std::cout << "Uso: competicion [--hilos H] [--veces V] [--comprobar]\n";
            return 2;
        }
    }
    if (hilos < 1 || veces < 1) {
        std::cout << "--hilos y --veces tienen que ser al menos 1\n";
        return 2;
    }

    std::vector<double> tiempos;
    std::uint64_t resultado = 0;
    for (int v = 0; v < veces; ++v) {
        auto inicio = std::chrono::steady_clock::now();
        std::uint64_t r = producto(hilos);
        auto fin = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();
        tiempos.push_back(ms);
        if (v > 0 && r != resultado) {
            std::cout << "ERROR: el resultado cambia entre ejecuciones (" << resultado << " y " << r << ")\n";
            return 1;
        }
        resultado = r;
        std::cout << "medición " << (v + 1) << ": " << ms << " ms\n";
    }
    std::sort(tiempos.begin(), tiempos.end());
    std::cout << "resultado: " << resultado << '\n';
    std::cout << "hilos: " << hilos << "   mediana: " << tiempos.at(tiempos.size() / 2) << " ms\n";

    if (comprobar) {
        std::uint64_t correcto = 1;
        for (std::uint64_t i = 0; i < N_VALORES; ++i) {
            correcto = (correcto * valor(i)) % MODULO;
        }
        std::cout << "correcto:  " << correcto << (correcto == resultado ? "   OK\n" : "   DISTINTO\n");
        return correcto == resultado ? 0 : 1;
    }
    return 0;
}
