// Caso 2: Cocina de un restaurante.
// Este archivo es el director de orquesta: lee la configuracion, lanza todos los hilos,
// coordina el cierre ordenado de la cocina y comprueba que no se haya perdido ningun plato.

#include "Cocina.h"
#include "Config.h"
#include "Contexto.h"
#include "Estadisticas.h"
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
}