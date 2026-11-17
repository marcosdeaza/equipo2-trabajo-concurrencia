// Prueba de la clase Semaforo (apartado 1.4 del enunciado).
// Comprueba dos cosas:
//   1. Con 3 plazas, nunca entran mas de 3 hilos a la vez.
//      Lo hacemos con nuestro Semaforo y con std::counting_semaphore<3>.
//   2. Con 0 plazas, tryAcquireFor(50 ms) tiene que devolver false.

#include "Semaforo.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

const int NUM_HILOS = 12;
const int VECES_POR_HILO = 20;
const int PLAZAS = 3;

// Plantilla: sirve para Semaforo y para std::counting_semaphore,
// porque los dos tienen acquire() y release().
template <typename Sem>
bool probarConLimite(Sem& sem, const char* nombre) {
    // dentro: cuantos hilos estan ahora mismo en la zona protegida.
    // maximoVisto: el valor mas alto que ha llegado a tener dentro.
    std::atomic<int> dentro{0};
    std::atomic<int> maximoVisto{0};

    auto trabajo = [&]() {
        for (int i = 0; i < VECES_POR_HILO; ++i) {
            sem.acquire();

            int ahora = ++dentro;

            // Actualizamos el maximo. Si otro hilo lo cambia a la vez, reintentamos.
            int previo = maximoVisto.load();
            while (ahora > previo && !maximoVisto.compare_exchange_weak(previo, ahora)) {
            }

            // Simulamos el trabajo dentro: 300 microsegundos.
            std::this_thread::sleep_for(std::chrono::microseconds(300));

            --dentro;
            sem.release();
        }
    };

    std::vector<std::thread> hilos;
    for (int i = 0; i < NUM_HILOS; ++i) {
        hilos.emplace_back(trabajo);
    }
    for (auto& h : hilos) {
        h.join();
    }

    if (maximoVisto.load() > PLAZAS) {
        std::cout << nombre << ": FALLO, entraron " << maximoVisto.load() << " a la vez\n";
        return false;
    }
    std::cout << nombre << ": maximo dentro = " << maximoVisto.load() << ", bien\n";
    return true;
}

int main() {
    bool todoBien = true;

    // Parte 1: nuestro Semaforo y la version estandar, con 3 plazas.
    Semaforo miSemaforo(PLAZAS);
    todoBien = probarConLimite(miSemaforo, "Semaforo") && todoBien;

    std::counting_semaphore<PLAZAS> semaforoEstandar(PLAZAS);
    todoBien = probarConLimite(semaforoEstandar, "std::counting_semaphore") && todoBien;

    // Parte 2: con 0 plazas no se puede coger ninguna, asi que tiene que dar false.
    Semaforo vacio(0);
    if (vacio.tryAcquireFor(std::chrono::milliseconds(50))) {
        std::cout << "Semaforo con 0 plazas: FALLO, devolvio true\n";
        todoBien = false;
    } else {
        std::cout << "Semaforo con 0 plazas: false, bien\n";
    }

    std::counting_semaphore<PLAZAS> estandarVacio(0);
    if (estandarVacio.try_acquire_for(std::chrono::milliseconds(50))) {
        std::cout << "std::counting_semaphore con 0 plazas: FALLO, devolvio true\n";
        todoBien = false;
    } else {
        std::cout << "std::counting_semaphore con 0 plazas: false, bien\n";
    }

    if (todoBien) {
        std::cout << "OK\n";
        return 0;
    }
    return 1;
}
