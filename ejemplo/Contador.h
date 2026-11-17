#pragma once

#include "Contexto.h"

#include <mutex>

// Clase de ejemplo: un contador compartido protegido por un mutex privado.
class Contador {
public:
    Contador(const Contexto& ctx, int hilos);

    void incrementar(int hilo);  // con --fallo carrera, sin cerrojo
    long valor();

    // Método que ejecuta cada hilo del ejemplo.
    void bucleHilo(int hilo, int veces);

private:
    const Contexto& _ctx;
    bool _sinCerrojo;
    bool _inanicion;
    std::mutex _m;
    long _valor = 0;
    int _otrosTrabajando;  // hilos distintos del 0 que no han terminado
};
