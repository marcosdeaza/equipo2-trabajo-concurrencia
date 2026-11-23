#include "BufferMonitor.h"

#include "Estadisticas.h"

#include <chrono>

BufferMonitor::BufferMonitor(int capacidad) : _capacidad(capacidad) {}

void BufferMonitor::poner(int plato) {
    std::unique_lock<std::mutex> bloqueo(_m);
    // Si la cola está llena, esperamos a que algún emplatador saque un plato.
    _hayHueco.wait(bloqueo, [this] { return static_cast<int>(_cola.size()) < _capacidad; });

    _cola.push(plato);

    // Medimos la ocupación media tras insertar.
    _sumaOcupacion += static_cast<long>(_cola.size());
    ++_numInserciones;

    // Avisamos a un emplatador que esté esperando plato.
    _hayPlato.notify_one();
}

bool BufferMonitor::tomar(int& plato, Estadisticas* stats) {
    std::unique_lock<std::mutex> bloqueo(_m);

    // Si nos pasan estadísticas, medimos el tiempo que el emplatador espera por plato.
    if (stats != nullptr) {
        auto t0 = std::chrono::steady_clock::now();
        _hayPlato.wait(bloqueo, [this] { return !_cola.empty() || _cerrado; });
        auto t1 = std::chrono::steady_clock::now();
        stats->registrarEspera("emplatador (buffer 2 vacío)", t1 - t0);
    } else {
        _hayPlato.wait(bloqueo, [this] { return !_cola.empty() || _cerrado; });
    }

    if (_cola.empty()) {
        // Buffer cerrado y sin platos pendientes: el emplatador ha terminado.
        return false;
    }

    plato = _cola.front();
    _cola.pop();

    // Avisamos a los cocineros de que hay un hueco libre en el buffer 2.
    _hayHueco.notify_one();
    return true;
}

void BufferMonitor::cerrar() {
    std::lock_guard<std::mutex> bloqueo(_m);
    _cerrado = true;
    // Despertamos a todos los emplatadores para que comprueben que la cocina ha cerrado.
    _hayPlato.notify_all();
}

double BufferMonitor::ocupacionMedia() const {
    std::lock_guard<std::mutex> bloqueo(_m);
    if (_numInserciones == 0) {
        return 0.0;
    }
    return static_cast<double>(_sumaOcupacion) / _numInserciones;
}

