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

// Plato especial que dice "ya no hay más comandas".
const int FIN_PLATO = -1;
// Cocineros y utensilios: son 10 fijos en el enunciado.
const int COCINEROS = 10;

// Simulación completa del restaurante:
// Camareros, cocineros, utensilios, horno FIFO, emplatadores y jefe de cocina.
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

    // Métricas de ocupación media para el experimento.
    double ocupacionMediaBuffer1() const;
    double ocupacionMediaBuffer2() const;

private:
    void entrarHorno();
    void salirHorno();

    const Contexto& _ctx;
    BufferSemaforos _entrada;  // Buffer 1 (semáforos)
    BufferMonitor _salida;     // Buffer 2 (monitor)
    std::vector<std::unique_ptr<Semaforo>> _utensilios;

    int _plazasHorno;

    // Modos de fallo configurados por terminal.
    bool _falloCarrera = false;
    bool _falloInterbloqueo = false;
    bool _falloInanicion = false;

    // Estado del horno (Monitor - Regla 3).
    // Todo este estado solo se accede bajo el mutex _mHorno.
    int _enHorno = 0;
    bool _limpiando = false;
    long _siguienteTicket = 0;  // Turno asignado al llegar al horno (FIFO)
    long _ticketAtendido = 0;   // Turno que tiene derecho a entrar ahora mismo
    std::mutex _mHorno;
    std::condition_variable _cvHorno;

    std::atomic<bool> _parar{false};
};

