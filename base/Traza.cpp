#include "Traza.h"

#include <iostream>
#include <thread>

Traza::Traza(bool activa) : _activa(activa), _inicio(std::chrono::steady_clock::now()) {}

void Traza::evento(const std::string& quien, const std::string& texto) {
    if (!_activa) {
        return;
    }
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now() - _inicio).count() / 1000.0;
    std::lock_guard<std::mutex> lk(_m);
    std::cout << "[" << ms << " ms] [hilo " << std::this_thread::get_id() << "] "
              << quien << ": " << texto << '\n';
}
