#pragma once

// Agrupa los objetos auxiliares que usan casi todas las clases de la simulación.
// Es solo una estructura de datos (sin métodos), por eso no tiene .cpp.
class Config;
class Estadisticas;
class Traza;
class Vigilante;

struct Contexto {
    const Config& config;
    Estadisticas& estadisticas;
    Traza& traza;
    Vigilante& vigilante;
};
