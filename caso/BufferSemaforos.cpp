#include "BufferSemaforos.h"

BufferSemaforos::BufferSemaforos(int capacidad) : _huecos(capacidad) {}

void BufferSemaforos::poner(int plato) {
    // Si no hay hueco libre, esperamos aqui.
    _huecos.acquire();
    {
        std::lock_guard<std::mutex> bloqueo(_m);
        _cola.push(plato);
    }
    // Avisamos de que hay un plato mas.
    _platos.release();
}

int BufferSemaforos::tomar() {
    // Si no hay platos, esperamos aqui.
    _platos.acquire();
    int plato;
    {
        std::lock_guard<std::mutex> bloqueo(_m);
        plato = _cola.front();
        _cola.pop();
    }
    // Ya hay un hueco libre mas.
    _huecos.release();
    return plato;
}
