#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <ostream>
#include <string>
#include <vector>

// Estadísticas de la simulación y registro de invariantes rotos.
// Todos los métodos son seguros entre hilos.
class Estadisticas {
public:
    // Tiempo que un hilo de cierto tipo ha esperado por un recurso.
    void registrarEspera(const std::string& tipo, std::chrono::steady_clock::duration espera);
    // Métrica numérica acumulada (operaciones, movimientos, emergencias...).
    void sumar(const std::string& metrica, long cantidad);
    long valor(const std::string& metrica);
    // Invariante roto.
    void violacion(const std::string& descripcion);
    bool sinViolaciones();
    std::vector<std::string> violaciones();

    void imprimir(std::ostream& os);

private:
    struct Esperas {
        long cuantas = 0;
        double totalMs = 0.0;
        double maximoMs = 0.0;
    };

    std::mutex _m;
    std::map<std::string, Esperas> _esperas;
    std::map<std::string, long> _metricas;
    std::vector<std::string> _violaciones;
    long _violacionesTotales = 0;
};
