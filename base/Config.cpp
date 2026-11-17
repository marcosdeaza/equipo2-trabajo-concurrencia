#include "Config.h"

#include <algorithm>

Config::Config(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--", 0) != 0) {
            continue;
        }
        std::string clave = arg.substr(2);
        std::string valor = "1";
        if (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
            valor = argv[i + 1];
            ++i;
        }
        _valores[clave] = valor;
    }
}

int Config::entero(const std::string& clave, int defecto) const {
    auto it = _valores.find(clave);
    return it == _valores.end() ? defecto : std::stoi(it->second);
}

std::string Config::texto(const std::string& clave, const std::string& defecto) const {
    auto it = _valores.find(clave);
    return it == _valores.end() ? defecto : it->second;
}

bool Config::bandera(const std::string& clave) const {
    return _valores.find(clave) != _valores.end();
}

bool Config::fallo(const std::string& nombre) const {
    return texto("fallo", "") == nombre;
}

std::vector<std::string> Config::clavesDesconocidas(const std::vector<std::string>& validas) const {
    std::vector<std::string> desconocidas;
    for (const auto& par : _valores) {
        if (std::find(validas.begin(), validas.end(), par.first) == validas.end()) {
            desconocidas.push_back(par.first);
        }
    }
    return desconocidas;
}
