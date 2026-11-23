#pragma once

#include "Semaforo.h"

#include <mutex>
#include <queue>
#include <semaphore>

// Buffer 1 (cadena de platos hecha con semáforos - Regla 1).
// _huecos cuenta las plazas libres (Semaforo propio).
// _platos cuenta los platos que hay disponibles (std::counting_semaphore).
// La cola en sí se modifica únicamente bajo exclusión mutua con _m.
class BufferSemaforos {
public:
    explicit BufferSemaforos(int capacidad);

    void poner(int plato);
    int tomar();

    // Devuelve la ocupación media (calculada justo tras cada inserción).
    double ocupacionMedia() const;

private:
    Semaforo _huecos;
    std::counting_semaphore<1000> _platos{0};
    mutable std::mutex _m;
    std::queue<int> _cola;

    // Métricas para calcular la ocupación media del buffer.
    long _sumaOcupacion = 0;
    long _numInserciones = 0;
};

