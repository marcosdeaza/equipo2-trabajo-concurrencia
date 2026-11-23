#pragma once

#include <cstdint>

// Devuelve (valor(0) * valor(1) * ... * valor(N_VALORES - 1)) mod MODULO,
// usando como mucho "hilos" hilos. Es lo único que programáis en la competición.
std::uint64_t producto(int hilos);
