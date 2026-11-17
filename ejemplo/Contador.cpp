#include "Contador.h"

#include "Config.h"
#include "Estadisticas.h"
#include "Traza.h"
#include "Vigilante.h"

#include <chrono>
#include <string>
#include <thread>

Contador::Contador(const Contexto& ctx, int hilos)
    : _ctx(ctx),
      _sinCerrojo(ctx.config.fallo("carrera")),
      _inanicion(ctx.config.fallo("inanicion")),
      _otrosTrabajando(hilos - 1) {}

void Contador::incrementar(int hilo) {
    if (_sinCerrojo) {
        long visto = _valor;  // ERROR deliberado: leer-modificar-escribir sin cerrojo
        std::this_thread::yield();
        _valor = visto + 1;
        return;
    }
    // La espera del hilo 0 se anota aparte para poder compararla con la de los demás.
    const std::string tipo = hilo == 0 ? "hilo 0" : "otros hilos";
    auto inicio = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lk(_m);
    if (_inanicion && hilo == 0) {
        // ERROR deliberado (--fallo inanicion): el hilo 0 cede siempre el turno.
        // Mientras quede otro hilo trabajando, suelta el cerrojo, se aparta 1 ms y vuelve a mirar.
        while (_otrosTrabajando > 0) {
            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            lk.lock();
        }
    }
    _ctx.estadisticas.registrarEspera(tipo, std::chrono::steady_clock::now() - inicio);
    ++_valor;
}

long Contador::valor() {
    std::lock_guard<std::mutex> lk(_m);
    return _valor;
}

void Contador::bucleHilo(int hilo, int veces) {
    for (int i = 0; i < veces; ++i) {
        incrementar(hilo);
        _ctx.vigilante.progreso();
        std::this_thread::sleep_for(std::chrono::microseconds(100));  // simula el resto del trabajo
    }
    if (hilo != 0) {
        std::lock_guard<std::mutex> lk(_m);
        --_otrosTrabajando;
    }
    _ctx.estadisticas.sumar("incrementos", veces);
    _ctx.traza.evento("hilo " + std::to_string(hilo), "termina");
}
