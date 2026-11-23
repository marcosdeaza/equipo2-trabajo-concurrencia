#include "Cocina.h"

#include "Config.h"
#include "Estadisticas.h"
#include "Traza.h"
#include "Vigilante.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

// Espera "ms" milisegundos para simular el tiempo de cada actividad según el enunciado.
static void esperar(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

Cocina::Cocina(const Contexto& ctx, int cola, int plazasHorno)
    : _ctx(ctx),
      _entrada(cola),
      _salida(cola),
      _plazasHorno(plazasHorno),
      _falloCarrera(ctx.config.fallo("carrera")),
      _falloInterbloqueo(ctx.config.fallo("interbloqueo")),
      _falloInanicion(ctx.config.fallo("inanicion")) {
    // Un semáforo contador por cada utensilio (10 en total).
    // Inicializado a 1: cada utensilio lo usa un solo cocinero a la vez (exclusión mutua).
    for (int i = 0; i < COCINEROS; ++i) {
        _utensilios.push_back(std::make_unique<Semaforo>(1));
    }
}

void Cocina::camarero(int numero, int comandas) {
    for (int i = 0; i < comandas; ++i) {
        // Medimos el tiempo que el camarero espera si el buffer 1 está lleno.
        auto t0 = std::chrono::steady_clock::now();
        _entrada.poner(numero);
        auto t1 = std::chrono::steady_clock::now();
        _ctx.estadisticas.registrarEspera("camarero (buffer 1 lleno)", t1 - t0);

        _ctx.estadisticas.sumar("platos pedidos", 1);
        _ctx.vigilante.progreso();
    }
}

void Cocina::cocinero(int numero) {
    int primero;
    int segundo;

    if (_falloInterbloqueo) {
        // MODO FALLO: INTERBLOQUEO (Tema 8).
        // Cada cocinero intenta coger primero su utensilio izquierdo y luego el derecho.
        // Al esperar 1 ms entre ambos, todos retienen el suyo y esperan el del vecino.
        // Se crea un ciclo de espera circular y el sistema se interbloquea.
        primero = numero;
        segundo = (numero + 1) % COCINEROS;
    } else {
        // VERSIÓN CORRECTA: ORDEN GLOBAL DE RECURSOS (Tema 8).
        // Todos los cocineros adquieren primero el utensilio de menor índice.
        // Esto rompe la condición de espera circular y hace imposible el deadlock.
        int a = numero;
        int b = (numero + 1) % COCINEROS;
        primero = std::min(a, b);
        segundo = std::max(a, b);
    }

    while (true) {
        int plato = _entrada.tomar();
        if (plato == FIN_PLATO) {
            break;  // Este cocinero ha terminado su jornada.
        }

        // Adquisición de utensilios midiendo la espera en semáforos:
        auto t0 = std::chrono::steady_clock::now();
        _utensilios[primero]->acquire();
        auto t1 = std::chrono::steady_clock::now();

        esperar(1);  // 1 ms entre coger el primer utensilio y el segundo

        auto t2 = std::chrono::steady_clock::now();
        _utensilios[segundo]->acquire();
        auto t3 = std::chrono::steady_clock::now();

        _ctx.estadisticas.registrarEspera("cocinero (utensilios)", (t1 - t0) + (t3 - t2));

        esperar(2);  // Cocinar en el fuego dura 2 ms

        // Liberamos ambos utensilios para los compañeros vecinos.
        _utensilios[primero]->release();
        _utensilios[segundo]->release();

        // Horneado: entrar en orden FIFO, esperar 3 ms y salir.
        entrarHorno();
        esperar(3);
        salirHorno();

        // Colocar el plato horneado en el buffer 2.
        _salida.poner(plato);
        _ctx.vigilante.progreso();
    }
}

void Cocina::emplatador() {
    int plato;
    // Toma platos midiendo la espera en caso de buffer vacío.
    while (_salida.tomar(plato, const_cast<Estadisticas*>(&_ctx.estadisticas))) {
        esperar(1);  // Emplatar dura 1 ms
        _ctx.estadisticas.sumar("platos emplatados", 1);
        _ctx.vigilante.progreso();
    }
}

void Cocina::jefe(int limpiezaMs) {
    while (!_parar) {
        esperar(limpiezaMs);

        // FASE 1: Espera a que el horno se vacíe completamente.
        {
            std::unique_lock<std::mutex> bloqueo(_mHorno);
            if (!_falloInanicion) {
                // Correcto: el jefe pone el cartel de limpieza ANTES de esperar.
                // Así ningún plato nuevo entra y el horno se vacía enseguida.
                _limpiando = true;
            }

            auto t0 = std::chrono::steady_clock::now();
            _cvHorno.wait(bloqueo, [this] { return _enHorno == 0; });
            auto t1 = std::chrono::steady_clock::now();
            _ctx.estadisticas.registrarEspera("jefe (horno vacío)", t1 - t0);

            // Comprobación de invariante: no puede haber platos si se va a limpiar.
            if (_enHorno > 0) {
                _ctx.estadisticas.violacion("el jefe limpia con platos dentro del horno (" +
                                            std::to_string(_enHorno) + ")");
            }

            if (_falloInanicion) {
                _limpiando = true;
            }
        } // RAII: el mutex se libera aquí automáticamente sin llamadas manuales a unlock()

        esperar(5);  // Limpiar el horno dura 5 ms (se hace sin bloquear el mutex)

        // FASE 2: Reapertura del horno.
        {
            std::lock_guard<std::mutex> bloqueo(_mHorno);
            _limpiando = false;
            _ctx.estadisticas.sumar("limpiezas", 1);
            // El jefe no llama a progreso(): según el enunciado el progreso
            // solo lo marcan los platos cocinados y emplatados.
        }
        // Despertamos a los cocineros que esperaban entrar al horno.
        _cvHorno.notify_all();
    }
}

void Cocina::ponerFinDeComandas() {
    // Un plato de fin por cada cocinero para que todos terminen ordenadamente.
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

double Cocina::ocupacionMediaBuffer1() const {
    return _entrada.ocupacionMedia();
}

double Cocina::ocupacionMediaBuffer2() const {
    return _salida.ocupacionMedia();
}

void Cocina::entrarHorno() {
    if (_falloCarrera) {
        // MODO FALLO: CONDICIÓN DE CARRERA.
        // Paso 1: comprobamos si hay plaza libre en una sección crítica.
        // Paso 2: entramos e incrementamos en otra sección crítica separada.
        // En el intervalo entre 1 y 2, otro cocinero se cuela y se supera el límite de plazas.
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
        esperar(1);  // Ventana de carrera

        std::lock_guard<std::mutex> bloqueo(_mHorno);
        ++_enHorno;
        if (_enHorno > _plazasHorno) {
            _ctx.estadisticas.violacion("más platos en el horno (" + std::to_string(_enHorno) +
                                        ") que plazas (" + std::to_string(_plazasHorno) + ")");
        }
        if (_limpiando) {
            _ctx.estadisticas.violacion("un plato ha entrado al horno mientras el jefe limpiaba");
        }
        return;
    }

    // VERSIÓN CORRECTA (u opción inanición):
    std::unique_lock<std::mutex> bloqueo(_mHorno);

    // Sistema de tickets para orden de llegada estricto (FIFO - Regla 3).
    long miTicket = _siguienteTicket++;

    auto t0 = std::chrono::steady_clock::now();
    if (_falloInanicion) {
        // MODO FALLO: INANICIÓN DEL JEFE.
        // Los cocineros ignoran _limpiando; siguen entrando y el jefe espera indefinidamente.
        _cvHorno.wait(bloqueo, [this, miTicket] {
            return miTicket == _ticketAtendido && _enHorno < _plazasHorno;
        });
    } else {
        // VERSIÓN CORRECTA:
        // Entra solo si es su turno (miTicket == _ticketAtendido),
        // el jefe no está limpiando/esperando (!_limpiando) y hay plaza libre (_enHorno < _plazasHorno).
        _cvHorno.wait(bloqueo, [this, miTicket] {
            return miTicket == _ticketAtendido && !_limpiando && _enHorno < _plazasHorno;
        });
    }
    auto t1 = std::chrono::steady_clock::now();
    _ctx.estadisticas.registrarEspera("cocinero (horno)", t1 - t0);

    // Ocupamos la plaza y avanzamos el turno atendido.
    ++_enHorno;
    ++_ticketAtendido;

    // Invariante 1: platos en el horno <= plazas_horno.
    if (_enHorno > _plazasHorno) {
        _ctx.estadisticas.violacion("más platos en el horno (" + std::to_string(_enHorno) +
                                    ") que plazas (" + std::to_string(_plazasHorno) + ")");
    }

    // Invariante 2: nunca hay platos dentro mientras se limpia.
    if (_limpiando) {
        _ctx.estadisticas.violacion("un plato ha entrado al horno mientras el jefe limpiaba");
    }

    // Notificamos para que el siguiente cocinero con ticket compruebe si cabe.
    _cvHorno.notify_all();
}

void Cocina::salirHorno() {
    {
        std::lock_guard<std::mutex> bloqueo(_mHorno);
        --_enHorno;
    }
    // Avisamos tanto al jefe (por si espera horno vacío) como al cocinero del siguiente ticket.
    _cvHorno.notify_all();
}

