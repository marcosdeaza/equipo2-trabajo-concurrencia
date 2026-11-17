#include "Valor.h"

// NO SE MODIFICA.
std::uint64_t valor(std::uint64_t i) {
    std::uint64_t base = (i * 2654435761ULL + 12345ULL) % MODULO;
    if (base == 0) {
        base = 1;
    }
    std::uint64_t vueltas = 1 + i / 2000;
    std::uint64_t r = 1;
    for (std::uint64_t k = 0; k < vueltas; ++k) {
        r = (r * base) % MODULO;
    }
    return r;
}
