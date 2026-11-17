#include "Semaforo.h"

// _valor es el numero de plazas libres.
// Solo lo tocamos con el mutex cogido, para que dos hilos no lo cambien a la vez.

Semaforo::Semaforo(int valorInicial) : _valor(valorInicial) {}

void Semaforo::acquire() {
    std::unique_lock<std::mutex> bloqueo(_m);

    // Si no hay plazas, el hilo se duerme aqui.
    // wait suelta el mutex mientras duerme, y lo vuelve a coger cuando lo despiertan.
    _cv.wait(bloqueo, [this] { return _valor > 0; });

    // Ya hay plaza: la cogemos.
    --_valor;
}

void Semaforo::release() {
    {
        // Soltamos la plaza con el mutex cogido.
        std::lock_guard<std::mutex> bloqueo(_m);
        ++_valor;
    }
    // Avisamos a uno que estuviera esperando en acquire.
    _cv.notify_one();
}

bool Semaforo::tryAcquireFor(std::chrono::milliseconds plazo) {
    std::unique_lock<std::mutex> bloqueo(_m);

    // Esperamos como mucho el tiempo "plazo".
    // Si se acaba el tiempo y sigue sin haber plaza, devolvemos false sin coger nada.
    if (!_cv.wait_for(bloqueo, plazo, [this] { return _valor > 0; })) {
        return false;
    }

    --_valor;
    return true;
}
