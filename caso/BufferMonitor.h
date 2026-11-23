#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// Declaración adelantada para no incluir Estadisticas.h si no es necesario.
class Estadisticas;

// Buffer 2 (monitor - Regla 1): una cola protegida con su mutex y variables de condición.
// Los platos salen exactamente en el mismo orden en que entraron (FIFO).
class BufferMonitor {
public:
    explicit BufferMonitor(int capacidad);

    void poner(int plato);
    // Devuelve false si el buffer está cerrado y ya no quedan platos pendientes.
    // Opcionalmente mide el tiempo de espera por plato si se le pasan las estadísticas.
    bool tomar(int& plato, Estadisticas* stats = nullptr);
    void cerrar();

    // Devuelve la ocupación media (calculada justo tras cada inserción).
    double ocupacionMedia() const;

private:
    int _capacidad;
    std::queue<int> _cola;
    bool _cerrado = false;
    mutable std::mutex _m;
    std::condition_variable _hayHueco;
    std::condition_variable _hayPlato;

    // Métricas para calcular la ocupación media.
    long _sumaOcupacion = 0;
    long _numInserciones = 0;
};

