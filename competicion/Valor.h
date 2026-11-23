#pragma once

#include <cstdint>

// Módulo de todas las operaciones de la competición (un número primo).
constexpr std::uint64_t MODULO = 1000000007ULL;

// Número de valores: se calcula el producto de valor(0) ... valor(N_VALORES - 1).
constexpr std::uint64_t N_VALORES = 2000000ULL;

// Devuelve el valor i (entre 1 y MODULO - 1). NO SE MODIFICA.
// Hay que llamarla exactamente una vez por cada i.
std::uint64_t valor(std::uint64_t i);
