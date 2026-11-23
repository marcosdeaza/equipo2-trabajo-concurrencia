// Caso 2: Cocina de un restaurante.
// Este archivo es el director de orquesta: lee la configuracion, lanza todos los hilos,
// coordina el cierre ordenado de la cocina y comprueba que no se haya perdido ningun plato.

#include "Cocina.h"
#include "Config.h"
#include "Contexto.h"
#include "Estadisticas.h"
#include "Lanzador.h"
#include "Traza.h"
#include "Vigilante.h"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

// Funcion principal de la simulacion que ejecuta el Lanzador una vez (o varias con --estres N).
// Devuelve true si la simulacion termina sin romper ningun invariante.
bool ejecutarSimulacion(const Config& config, Estadisticas& estadisticas, Traza& traza, const std::string& etiqueta) {
    // 1. Leemos los parametros pasados por terminal (o usamos los valores por defecto del enunciado).
    int camareros     = config.entero("camareros", 4);       // Cuantos camareros toman comandas (4).
    int comandas      = config.entero("comandas", 30);      // Cuantos platos pide cada camarero (30).
    int cola          = config.entero("cola", 4);           // Capacidad maxima de los buffers 1 y 2 (4).
    int plazasHorno   = config.entero("plazas_horno", 3);   // Platos que caben a la vez en el horno (3).
    int emplatadores  = config.entero("emplatadores", 2);   // Cuantos emplatadores sacan platos (2).
    int limpieza      = config.entero("limpieza", 20);      // Cada cuantos ms limpia el horno el jefe (20 ms).

    // 2. Preparamos el vigilante (si pasan 2000 ms sin avance, avisa de posible bloqueo y termina con codigo 3).
    Vigilante vigilante(std::chrono::milliseconds(config.entero("vigilante", 2000)), estadisticas, etiqueta);

    // 3. Empaquetamos configuracion, estadisticas, traza y vigilante en un unico Contexto para la cocina.
    Contexto ctx{config, estadisticas, traza, vigilante};

    // 4. Creamos la cocina con sus dos buffers (capacidad 'cola') y su horno ('plazasHorno').
    Cocina cocina(ctx, cola, plazasHorno);

    // Arrancamos el hilo vigilante para supervisar que la simulacion avance.
    vigilante.iniciar();

    // 5. Lanzamos el hilo del jefe de cocina (limpia el horno periodicamente).
    std::thread hiloJefe(&Cocina::jefe, &cocina, limpieza);

    // 6. Lanzamos los hilos de los camareros (cada uno genera sus comandas y las mete al buffer 1).
    std::vector<std::thread> hilosCamareros;
    for (int c = 0; c < camareros; ++c) {
        hilosCamareros.emplace_back(&Cocina::camarero, &cocina, c, comandas);
    }

    // 7. Lanzamos los 10 cocineros (sacan platos del buffer 1, usan sarten/fogon, hornean y pasan al buffer 2).
    std::vector<std::thread> hilosCocineros;
    for (int i = 0; i < COCINEROS; ++i) {
        hilosCocineros.emplace_back(&Cocina::cocinero, &cocina, i);
    }

    // 8. Lanzamos los hilos emplatadores (sacan platos listos del buffer 2 y los emplatan).
    std::vector<std::thread> hilosEmplatadores;
    for (int e = 0; e < emplatadores; ++e) {
        hilosEmplatadores.emplace_back(&Cocina::emplatador, &cocina);
    }

    // =========================================================================
    // FASE DE CIERRE ORDENADO:
    // El orden es vital: si esperamos a una etapa antes de que acabe la anterior,
    // o no avisamos del fin, los hilos se quedarian esperando para siempre.
    // =========================================================================

    // Paso 1: Esperamos a que todos los camareros hayan terminado de pedir sus comandas.
    for (auto& t : hilosCamareros) {
        t.join();
    }

    // Paso 2: Ya no habra mas comandas normales. Metemos un plato especial FIN_PLATO por cocinero (10).
    cocina.ponerFinDeComandas();

    // Paso 3: Esperamos a que los cocineros cocinen todo lo pendiente y terminen al ver su FIN_PLATO.
    for (auto& t : hilosCocineros) {
        t.join();
    }

    // Paso 4: Ya no se van a cocinar mas platos. Cerramos el buffer 2 y despertamos a los emplatadores.
    cocina.cerrarSalida();

    // Paso 5: Esperamos a que los emplatadores terminen de vaciar el buffer 2 y salgan.
    for (auto& t : hilosEmplatadores) {
        t.join();
    }

    // Paso 6: Pedimos al jefe de cocina que pare sus rondas de limpieza y esperamos a que termine.
    cocina.pararJefe();
    hiloJefe.join();

    // Paso 7: La simulación ha terminado con éxito, paramos el vigilante.
    vigilante.detener();

    // =========================================================================
    // REGISTRO DE MÉTRICAS ADICIONALES (OCUPACIÓN MEDIA):
    // Como Estadisticas::sumar solo admite enteros, guardamos la media en centésimas (x100).
    // Ejemplo: valor 385 significa una ocupación media de 3.85 platos.
    // =========================================================================
    long media1Cent = std::lround(cocina.ocupacionMediaBuffer1() * 100.0);
    long media2Cent = std::lround(cocina.ocupacionMediaBuffer2() * 100.0);
    estadisticas.sumar("ocupación media buffer 1 (x100)", media1Cent);
    estadisticas.sumar("ocupación media buffer 2 (x100)", media2Cent);

    // =========================================================================
    // COMPROBACION DE INVARIANTES FINALES:
    // Verificamos que los numeros cuadren exactamente y no se haya perdido nada.
    // =========================================================================
    long pedidos = estadisticas.valor("platos pedidos");
    long emplatados = estadisticas.valor("platos emplatados");
    long esperado = static_cast<long>(camareros) * comandas;

    // Invariante 1: los platos pedidos deben coincidir con camareros x comandas.
    if (pedidos != esperado) {
        estadisticas.violacion("platos pedidos = " + std::to_string(pedidos) + ", esperado " +
                               std::to_string(esperado));
    }

    // Invariante 2: todos los platos pedidos deben haber sido emplatados sin faltar ninguno.
    if (emplatados != pedidos) {
        estadisticas.violacion("platos emplatados = " + std::to_string(emplatados) + ", pedidos " +
                               std::to_string(pedidos));
    }

    // Devuelve true si no hubo ninguna violacion de invariantes.
    return estadisticas.sinViolaciones();
}

int main(int argc, char* argv[]) {
    // Lee los argumentos pasados por terminal (--clave valor).
    Config config(argc, argv);

    // Configuramos el Lanzador con las opciones permitidas y los tres modos de fallo.
    Lanzador lanzador("caso",
                      {"camareros", "comandas", "cola", "plazas_horno", "emplatadores", "limpieza"},
                      {"carrera", "interbloqueo", "inanicion"});

    // Ejecuta la simulacion (gestiona --estres N, imprime resultados y devuelve codigo 0 o 1).
    return lanzador.ejecutar(config, ejecutarSimulacion);
}
