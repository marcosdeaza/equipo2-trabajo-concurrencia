#include "Cocina.h"

#include "Config.h"
#include "Estadisticas.h"
#include "Traza.h"
#include "Vigilante.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

// Espera "ms" milisegundos. Sirve para simular el tiempo que dura cada cosa.
static void esperar(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

Cocina::Cocina(const Contexto& ctx, int cola, int plazasHorno)
    : _ctx(ctx),
      _entrada(cola),
      _salida(cola),
      _plazasHorno(plazasHorno),
      _sinCerrojo(ctx.config.fallo("carrera")) {
    // Un semaforo por utensilio. Cada uno vale 1: lo usa un cocinero cada vez.
    for (int i = 0; i < COCINEROS; ++i) {
        _utensilios.push_back(std::make_unique<Semaforo>(1));
    }
}

void Cocina::camarero(int numero, int comandas) {
    for (int i = 0; i < comandas; ++i) {
        // El valor del plato no importa, solo que no sea FIN_PLATO.
        _entrada.poner(numero);
        _ctx.estadisticas.sumar("platos pedidos", 1);
        _ctx.vigilante.progreso();
    }
}

void Cocina::cocinero(int numero) {
    // El cocinero i usa el utensilio i y el (i+1) mod 10.
    int a = numero;
    int b = (numero + 1) % COCINEROS;
    // Para no quedarnos bloqueados, cogemos siempre primero el de numero mas bajo.
    int primero = std::min(a, b);
    int segundo = std::max(a, b);

    while (true) {
        int plato = _entrada.tomar();
        if (plato == FIN_PLATO) {
            break;  // este cocinero ya no tiene nada que hacer
        }

        // Cogemos los dos utensilios (con 1 ms entre uno y otro).
        _utensilios[primero]->acquire();
        esperar(1);
        _utensilios[segundo]->acquire();

        esperar(2);  // fuego

        // Soltamos los utensilios para que los use el vecino.
        _utensilios[primero]->release();
        _utensilios[segundo]->release();

        // Horno: 3 ms dentro.
        entrarHorno();
        esperar(3);
        salirHorno();

        _salida.poner(plato);
        _ctx.vigilante.progreso();
    }
}

void Cocina::emplatador() {
    int plato;
    while (_salida.tomar(plato)) {
        esperar(1);  // emplatar dura 1 ms
        _ctx.estadisticas.sumar("platos emplatados", 1);
        _ctx.vigilante.progreso();
    }
}

void Cocina::jefe(int limpiezaMs) {
    while (!_parar) {
        esperar(limpiezaMs);

        std::unique_lock<std::mutex> bloqueo(_mHorno);
        // Cerramos la puerta del horno: ya no entra nadie.
        _limpiando = true;
        // Esperamos a que salgan los que estan dentro.
        _cvHorno.wait(bloqueo, [this] { return _enHorno == 0; });

        esperar(5);  // limpiar dura 5 ms

        _limpiando = false;
        _ctx.estadisticas.sumar("limpiezas", 1);
        _ctx.vigilante.progreso();
        bloqueo.unlock();
        _cvHorno.notify_all();
    }
}

void Cocina::ponerFinDeComandas() {
    // Un plato de fin para cada cocinero.
    for (int i = 0; i < COCINEROS; ++i) {
        _entrada.poner(FIN_PLATO);
    }
}

void Cocina::cerrarSalida() {
    _salida.cerrar();
}

void Cocina::pararJefe() {
    _parar = true;
}

void Cocina::entrarHorno() {
    if (_sinCerrojo) {
        // ERROR a proposito (--fallo carrera).
        // Paso 1: miramos si hay sitio (con el mutex, pero solo para mirar).
        // Paso 2: despues de mirar, entramos. Entre un paso y otro otro cocinero
        // puede mirar lo mismo y entrar tambien: ahi esta la carrera.
        bool hayHueco = false;
        while (!hayHueco) {
            {
                std::lock_guard<std::mutex> bloqueo(_mHorno);
                hayHueco = _enHorno < _plazasHorno;
            }
            if (!hayHueco) {
                esperar(1);
            }
        }
        esperar(1);  // el tiempo entre mirar y entrar

        std::lock_guard<std::mutex> bloqueo(_mHorno);
        ++_enHorno;
        if (_enHorno > _plazasHorno) {
            _ctx.estadisticas.violacion("mas platos en el horno (" + std::to_string(_enHorno) +
                                        ") que plazas (" + std::to_string(_plazasHorno) + ")");
        }
        return;
    }

    // Version buena: esperamos con el mutex y la variable de condicion.
    std::unique_lock<std::mutex> bloqueo(_mHorno);
    _cvHorno.wait(bloqueo, [this] { return !_limpiando && _enHorno < _plazasHorno; });
    ++_enHorno;
}

void Cocina::salirHorno() {
    {
        std::lock_guard<std::mutex> bloqueo(_mHorno);
        --_enHorno;
    }
    _cvHorno.notify_all();
}
