#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

// Semáforo contador implementado como monitor (requisito 5).
// La interfaz es obligatoria; la implementación es vuestra (Semaforo.cpp).
class Semaforo {
public:
    explicit Semaforo(int valorInicial);

    void acquire();
    void release();
    // Devuelve false si no se ha podido adquirir antes de que venza el plazo.
    bool tryAcquireFor(std::chrono::milliseconds plazo);

private:
    std::mutex _m;
    std::condition_variable _cv;
    int _valor;
};
