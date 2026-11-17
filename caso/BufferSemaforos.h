#pragma once

#include "Semaforo.h"

#include <mutex>
#include <queue>
#include <semaphore>

// Buffer 1 (cadena de platos hecha con semaforos).
// _huecos cuenta las plazas libres, _platos cuenta los platos que hay.
// La cola en si se toca solo con el mutex cogido.
class BufferSemaforos {
public:
    explicit BufferSemaforos(int capacidad);

    void poner(int plato);
    int tomar();

private:
    Semaforo _huecos;
    std::counting_semaphore<1000> _platos{0};
    std::mutex _m;
    std::queue<int> _cola;
};
