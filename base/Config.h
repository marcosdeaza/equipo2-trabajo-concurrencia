#pragma once

#include <map>
#include <string>
#include <vector>

// Configuración leída de la línea de órdenes: --clave valor  o  --bandera.
// Solo se lee después de construirla, así que se puede consultar desde varios hilos.
class Config {
public:
    Config(int argc, char* argv[]);

    int entero(const std::string& clave, int defecto) const;
    std::string texto(const std::string& clave, const std::string& defecto) const;
    bool bandera(const std::string& clave) const;
    // ¿Está activo el modo --fallo <nombre>?
    bool fallo(const std::string& nombre) const;

    // Devuelve las claves recibidas que no están en la lista (para avisar de erratas).
    std::vector<std::string> clavesDesconocidas(const std::vector<std::string>& validas) const;

private:
    std::map<std::string, std::string> _valores;
};
