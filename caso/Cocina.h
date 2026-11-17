#pragma once

#include "BufferMonitor.h"
#include "BufferSemaforos.h"
#include "Contexto.h"
#include "Semaforo.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

// Plato especial que dice "ya no hay mas comandas".
const int FIN_PLATO = -1;
// Cocineros y utensilios: son fijos en el enunciado.
const int COCINEROS = 10;

// Todo el restaurante: camareros, cocineros, horno, emplatadores y jefe de cocina.
class Cocina {
public:
    Cocina(const Contexto& ctx, int cola, int plazasHorno);

    void camarero(int numero, int comandas);
    void cocinero(int numero);
    void emplatador();
    void jefe(int limpiezaMs);

    void ponerFinDeComandas();
    void cerrarSalida();
    void pararJefe();

private:
    void entrarHorno();
    void salirHorno();

    const Contexto& _ctx;
    BufferSemaforos _entrada;  // buffer 1
    BufferMonitor _salida;     // buffer 2
    std::vector<std::unique_ptr<Semaforo>> _utensilios;

    int _plazasHorno;
    bool _sinCerrojo;  // true con --fallo carrera

    // Estado del horno. Solo se toca con _mHorno cogido (salvo en el modo carrera).
    int _enHorno = 0;
    bool _limpiando = false;
    std::mutex _mHorno;
    std::condition_variable _cvHorno;

    std::atomic<bool> _parar{false};
};
