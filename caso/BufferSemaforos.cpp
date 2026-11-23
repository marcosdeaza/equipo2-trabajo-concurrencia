#include "BufferSemaforos.h"

BufferSemaforos::BufferSemaforos(int capacidad) : _huecos(capacidad) {}

void BufferSemaforos::poner(int plato) {
    // Si el buffer está lleno, esperamos aquí hasta que haya un hueco libre.
    _huecos.acquire();

    {
        // Modificamos la cola en exclusión mutua para evitar carreras.
        std::lock_guard<std::mutex> bloqueo(_m);
        _cola.push(plato);

        // Medimos la ocupación justo después de insertar (para el experimento).
        _sumaOcupacion += static_cast<long>(_cola.size());
        ++_numInserciones;
    }

    // Avisamos a los cocineros de que hay un plato más listo para cocinar.
    _platos.release();
}

int BufferSemaforos::tomar() {
    // Si no hay platos disponibles, esperamos aquí a que un camarero ponga uno.
    _platos.acquire();

    int plato;
    {
        // Sacamos el plato en exclusión mutua.
        std::lock_guard<std::mutex> bloqueo(_m);
        plato = _cola.front();
        _cola.pop();
    }

    // Ya hemos liberado un hueco, avisamos al semáforo de huecos.
    _huecos.release();
    return plato;
}

double BufferSemaforos::ocupacionMedia() const {
    std::lock_guard<std::mutex> bloqueo(_m);
    if (_numInserciones == 0) {
        return 0.0;
    }
    return static_cast<double>(_sumaOcupacion) / _numInserciones;
}

