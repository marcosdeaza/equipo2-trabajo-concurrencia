#pragma once

#include <chrono>
#include <mutex>
#include <string>

// Traza de eventos opcional (--traza). Cada línea: tiempo, hilo, quién y qué.
// La escritura está protegida por un mutex para que no se mezclen las líneas.
class Traza {
public:
    explicit Traza(bool activa);

    void evento(const std::string& quien, const std::string& texto);
    bool activa() const { return _activa; }

private:
    bool _activa;
    std::chrono::steady_clock::time_point _inicio;
    std::mutex _m;
};
