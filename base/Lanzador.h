#pragma once

#include <string>
#include <vector>

class Config;
class Estadisticas;
class Traza;

// Función que ejecuta UNA simulación completa. Devuelve true si se cumplen los invariantes.
using FuncionSimulacion = bool (*)(const Config& config, Estadisticas& estadisticas,
                                   Traza& traza, const std::string& etiqueta);

// Lógica común de main: validar opciones, ejecución única o --estres N.
class Lanzador {
public:
    Lanzador(std::string nombrePrograma, std::vector<std::string> opciones, std::vector<std::string> fallos);

    int ejecutar(const Config& config, FuncionSimulacion simulacion);

private:
    void ayuda();

    std::string _nombre;
    std::vector<std::string> _opciones;
    std::vector<std::string> _fallos;
};
