// Ejemplo de uso del proyecto base: 4 hilos incrementan un contador compartido.
//   ./ejemplo                         una ejecución
//   ./ejemplo --traza                 con traza
//   ./ejemplo --hilos 8 --veces 5000  cambiando la configuración
//   ./ejemplo --fallo carrera --estres 10
//   ./ejemplo --fallo inanicion       el hilo 0 espera mucho más que los demás

#include "Config.h"
#include "Contador.h"
#include "Contexto.h"
#include "Estadisticas.h"
#include "Lanzador.h"
#include "Traza.h"
#include "Vigilante.h"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

bool ejecutarSimulacion(const Config& config, Estadisticas& estadisticas, Traza& traza, const std::string& etiqueta) {
    int hilos = config.entero("hilos", 4);
    int veces = config.entero("veces", 1000);

    // El vigilante se crea en cada simulación; --vigilante ms cambia su límite.
    Vigilante vigilante(std::chrono::milliseconds(config.entero("vigilante", 2000)), estadisticas, etiqueta);
    Contexto ctx{config, estadisticas, traza, vigilante};
    Contador contador(ctx, hilos);

    vigilante.iniciar();
    std::vector<std::thread> trabajadores;
    for (int h = 0; h < hilos; ++h) {
        trabajadores.emplace_back(&Contador::bucleHilo, &contador, h, veces);
    }
    for (auto& t : trabajadores) {
        t.join();
    }
    vigilante.detener();

    // Invariante final.
    long esperado = static_cast<long>(hilos) * veces;
    if (contador.valor() != esperado) {
        estadisticas.violacion("contador = " + std::to_string(contador.valor()) + ", esperado " +
                               std::to_string(esperado));
    }
    return estadisticas.sinViolaciones();
}

int main(int argc, char* argv[]) {
    Config config(argc, argv);
    Lanzador lanzador("ejemplo", {"hilos", "veces"}, {"carrera", "inanicion"});
    return lanzador.ejecutar(config, ejecutarSimulacion);
}
