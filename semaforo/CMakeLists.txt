# Vuestra clase Semaforo, como biblioteca: la usan el caso y prueba_semaforo.
add_library(semaforo STATIC
    Semaforo.h
    Semaforo.cpp
)
target_include_directories(semaforo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(semaforo PUBLIC Threads::Threads)
