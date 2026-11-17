#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// Buffer 2 (monitor): una cola con su mutex y dos variables de condicion.
// Los platos salen en el mismo orden en que entraron.
class BufferMonitor {
public:
    explicit BufferMonitor(int capacidad);

    void poner(int plato);
    // Devuelve false si el buffer esta cerrado y ya no quedan platos.
    bool tomar(int& plato);
    void cerrar();

private:
    int _capacidad;
    std::queue<int> _cola;
    bool _cerrado = false;
    std::mutex _m;
    std::condition_variable _hayHueco;
    std::condition_variable _hayPlato;
};
