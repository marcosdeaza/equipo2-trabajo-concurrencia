#include "BufferMonitor.h"

BufferMonitor::BufferMonitor(int capacidad) : _capacidad(capacidad) {}

void BufferMonitor::poner(int plato) {
    std::unique_lock<std::mutex> bloqueo(_m);
    // Si la cola esta llena, esperamos a que alguien saque un plato.
    _hayHueco.wait(bloqueo, [this] { return static_cast<int>(_cola.size()) < _capacidad; });
    _cola.push(plato);
    _hayPlato.notify_one();
}

bool BufferMonitor::tomar(int& plato) {
    std::unique_lock<std::mutex> bloqueo(_m);
    // Esperamos mientras no haya platos y el buffer siga abierto.
    _hayPlato.wait(bloqueo, [this] { return !_cola.empty() || _cerrado; });
    if (_cola.empty()) {
        // Cerrado y sin platos: ya no hay nada mas que hacer.
        return false;
    }
    plato = _cola.front();
    _cola.pop();
    _hayHueco.notify_one();
    return true;
}

void BufferMonitor::cerrar() {
    std::lock_guard<std::mutex> bloqueo(_m);
    _cerrado = true;
    // Despertamos a todos los que esperan, para que vean que esta cerrado.
    _hayPlato.notify_all();
}
