#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

class Estadisticas;

// Hilo auxiliar que detecta falta de progreso (interbloqueo, livelock o despertar perdido).
// Los hilos de la simulación llaman a progreso() cada vez que completan algo útil.
// Si pasa el límite sin progreso, imprime las estadísticas y termina el proceso:
// un hilo interbloqueado no se puede despertar ni hacerle join.
class Vigilante {
public:
    Vigilante(std::chrono::milliseconds limite, Estadisticas& estadisticas, const std::string& etiqueta);
    ~Vigilante();

    void iniciar();
    void progreso();
    void detener();

private:
    void vigilar();

    std::chrono::milliseconds _limite;
    Estadisticas& _estadisticas;
    std::string _etiqueta;
    std::mutex _m;
    std::condition_variable _cv;
    long _progresos = 0;
    bool _detenido = false;
    std::thread _hilo;
};
