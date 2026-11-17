#include "Estadisticas.h"

#include <iomanip>

void Estadisticas::registrarEspera(const std::string& tipo, std::chrono::steady_clock::duration espera) {
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(espera).count() / 1000.0;
    std::lock_guard<std::mutex> lk(_m);
    Esperas& e = _esperas[tipo];
    ++e.cuantas;
    e.totalMs += ms;
    if (ms > e.maximoMs) {
        e.maximoMs = ms;
    }
}

void Estadisticas::sumar(const std::string& metrica, long cantidad) {
    std::lock_guard<std::mutex> lk(_m);
    _metricas[metrica] += cantidad;
}

long Estadisticas::valor(const std::string& metrica) {
    std::lock_guard<std::mutex> lk(_m);
    auto it = _metricas.find(metrica);
    return it == _metricas.end() ? 0 : it->second;
}

void Estadisticas::violacion(const std::string& descripcion) {
    std::lock_guard<std::mutex> lk(_m);
    ++_violacionesTotales;
    // Solo se guardan las primeras, para no llenar la memoria en los modos de fallo.
    if (_violaciones.size() < 10) {
        _violaciones.push_back(descripcion);
    }
}

bool Estadisticas::sinViolaciones() {
    std::lock_guard<std::mutex> lk(_m);
    return _violacionesTotales == 0;
}

std::vector<std::string> Estadisticas::violaciones() {
    std::lock_guard<std::mutex> lk(_m);
    return _violaciones;
}

void Estadisticas::imprimir(std::ostream& os) {
    std::lock_guard<std::mutex> lk(_m);
    os << std::fixed << std::setprecision(2);
    os << "\n=== Esperas por tipo de hilo ===\n";
    for (const auto& par : _esperas) {
        const Esperas& e = par.second;
        os << "  " << std::left << std::setw(44) << par.first << std::right
           << " n=" << std::setw(6) << e.cuantas
           << "  media=" << std::setw(8) << (e.cuantas ? e.totalMs / e.cuantas : 0.0) << " ms"
           << "  max=" << std::setw(8) << e.maximoMs << " ms\n";
    }
    os << "=== Métricas ===\n";
    for (const auto& par : _metricas) {
        os << "  " << std::left << std::setw(44) << par.first << std::right << " " << par.second << '\n';
    }
    os << "=== Invariantes ===\n";
    if (_violacionesTotales == 0) {
        os << "  OK\n";
    } else {
        os << "  ROTOS: " << _violacionesTotales << " violaciones. Primeras:\n";
        for (const auto& v : _violaciones) {
            os << "   - " << v << '\n';
        }
    }
}
