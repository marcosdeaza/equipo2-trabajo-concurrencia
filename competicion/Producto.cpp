#include "Producto.h"

#include "Valor.h"

#include <cstddef>
#include <thread>
#include <vector>

// Calcula el producto acumulado módulo 1 000 000 007 de valor(0) hasta valor(N_VALORES - 1)
// utilizando concurrencia con std::thread y reparto de carga balanceado.
std::uint64_t producto(int hilos) {
    // Si se pide 1 hilo o un valor menor, ejecutamos el cálculo secuencial directo.
    if (hilos <= 1) {
        std::uint64_t r = 1;
        for (std::uint64_t i = 0; i < N_VALORES; ++i) {
            r = (r * valor(i)) % MODULO;
        }
        return r;
    }

    // Cada hilo almacena su propio producto parcial en su celda del vector.
    // Al ser índices disjuntos, no existe condición de carrera y no necesitamos mutexes.
    std::vector<std::uint64_t> parciales(static_cast<std::size_t>(hilos), 1);
    std::vector<std::thread> trabajadores;
    trabajadores.reserve(static_cast<std::size_t>(hilos));

    for (int h = 0; h < hilos; ++h) {
        trabajadores.emplace_back([h, hilos, &parciales]() {
            std::uint64_t prod = 1;
            // Reparto entrelazado (round-robin / stride):
            // El hilo h procesa los índices: h, h + hilos, h + 2*hilos, etc.
            // Esto balancea perfectamente el trabajo porque valor(i) realiza más
            // operaciones a medida que i aumenta (distribución no uniforme).
            for (std::uint64_t i = static_cast<std::uint64_t>(h); i < N_VALORES; i += static_cast<std::uint64_t>(hilos)) {
                prod = (prod * valor(i)) % MODULO;
            }
            parciales[static_cast<std::size_t>(h)] = prod;
        });
    }

    // Esperamos a que todos los hilos terminen sus respectivos cálculos.
    for (auto& t : trabajadores) {
        t.join();
    }

    // Combinamos los resultados parciales en el hilo principal.
    std::uint64_t total = 1;
    for (int h = 0; h < hilos; ++h) {
        total = (total * parciales[static_cast<std::size_t>(h)]) % MODULO;
    }

    return total;
}

