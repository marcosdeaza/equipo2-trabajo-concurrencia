# Caso 2: cocina de un restaurante.

set(NOMBRE_EJECUTABLE caso)

add_executable(${NOMBRE_EJECUTABLE}
    main.cpp
    Cocina.h
    Cocina.cpp
    BufferSemaforos.h
    BufferSemaforos.cpp
    BufferMonitor.h
    BufferMonitor.cpp
)
target_link_libraries(${NOMBRE_EJECUTABLE} PRIVATE base semaforo)
