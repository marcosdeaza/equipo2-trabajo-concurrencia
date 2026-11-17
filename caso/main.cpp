// Caso 2: cocina de un restaurante.
// main crea los objetos, lanza los hilos, hace el cierre y comprueba los invariantes.

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

bool ejecutarSimulacion(const Config& config, Estadisticas& estadisticas, Traza& traza, const std::string& etiqueta) {
    int camareros = config.entero("camareros", 4);
    int comandas = config.entero("comandas", 30);
    int cola = config.entero("cola", 4);
    int plazasHorno = config.entero("plazas_horno", 3);
    int emplatadores = config.entero("emplatadores", 2);
    int limpieza = config.entero("limpieza", 20);

    Vigilante vigilante(std::chrono::milliseconds(config.entero("vigilante", 2000)), estadisticas, etiqueta);
    Contexto ctx{config, estadisticas, traza, vigilante};
    Cocina cocina(ctx, cola, plazasHorno);

    vigilante.iniciar();

    // Lanzamos todos los hilos.
    std::thread hiloJefe(&Cocina::jefe, &cocina, limpieza);

    std::vector<std::thread> hilosCamareros;
    for (int c = 0; c < camareros; ++c) {
        hilosCamareros.emplace_back(&Cocina::camarero, &cocina, c, comandas);
    }

    std::vector<std::thread> hilosCocineros;
    for (int i = 0; i < COCINEROS; ++i) {
        hilosCocineros.emplace_back(&Cocina::cocinero, &cocina, i);
    }

    std::vector<std::thread> hilosEmplatadores;
    for (int e = 0; e < emplatadores; ++e) {
        hilosEmplatadores.emplace_back(&Cocina::emplatador, &cocina);
    }

    // Cierre, en orden: primero los que empiezan, luego los que siguen.
    for (auto& t : hilosCamareros) {
        t.join();
    }
    cocina.ponerFinDeComandas();
    for (auto& t : hilosCocineros) {
        t.join();
    }
    cocina.cerrarSalida();
    for (auto& t : hilosEmplatadores) {
        t.join();
    }
    cocina.pararJefe();
    hiloJefe.join();
    vigilante.detener();

    // Invariantes finales: todo lo pedido se ha emplatado.
    long pedidos = estadisticas.valor("platos pedidos");
    long emplatados = estadisticas.valor("platos emplatados");
    long esperado = static_cast<long>(camareros) * comandas;
    if (pedidos != esperado) {
        estadisticas.violacion("platos pedidos = " + std::to_string(pedidos) + ", esperado " +
                               std::to_string(esperado));
    }
    if (emplatados != pedidos) {
        estadisticas.violacion("platos emplatados = " + std::to_string(emplatados) + ", pedidos " +
                               std::to_string(pedidos));
    }
    return estadisticas.sinViolaciones();
}

int main(int argc, char* argv[]) {
    Config config(argc, argv);
    Lanzador lanzador("caso",
                      {"camareros", "comandas", "cola", "plazas_horno", "emplatadores", "limpieza"},
                      {"carrera"});
    return lanzador.ejecutar(config, ejecutarSimulacion);
}
