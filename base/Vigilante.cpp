#include "Vigilante.h"

#include "Estadisticas.h"

#include <cstdlib>
#include <iostream>

Vigilante::Vigilante(std::chrono::milliseconds limite, Estadisticas& estadisticas, const std::string& etiqueta)
    : _limite(limite), _estadisticas(estadisticas), _etiqueta(etiqueta) {}

Vigilante::~Vigilante() {
    detener();
}

void Vigilante::iniciar() {
    _hilo = std::thread(&Vigilante::vigilar, this);
}

void Vigilante::progreso() {
    std::lock_guard<std::mutex> lk(_m);
    ++_progresos;
}

void Vigilante::detener() {
    {
        std::lock_guard<std::mutex> lk(_m);
        _detenido = true;
    }
    _cv.notify_all();
    if (_hilo.joinable()) {
        _hilo.join();
    }
}

void Vigilante::vigilar() {
    bool sinProgreso = false;
    {
        std::unique_lock<std::mutex> lk(_m);
        long visto = _progresos;
        auto ultimoCambio = std::chrono::steady_clock::now();
        while (!_detenido && !sinProgreso) {
            _cv.wait_for(lk, std::chrono::milliseconds(50), [this] { return _detenido; });
            auto ahora = std::chrono::steady_clock::now();
            if (_progresos != visto) {
                visto = _progresos;
                ultimoCambio = ahora;
            } else if (!_detenido && ahora - ultimoCambio > _limite) {
                sinProgreso = true;
            }
        }
    }
    if (sinProgreso) {
        std::cout << "\n*** VIGILANTE (" << _etiqueta << "): sin progreso durante " << _limite.count()
                  << " ms. Posible interbloqueo, livelock o despertar perdido. ***\n";
        _estadisticas.imprimir(std::cout);
        std::cout << "Se termina el proceso: los hilos bloqueados no se pueden despertar.\n";
        std::cout.flush();
        std::_Exit(3);
    }
}
