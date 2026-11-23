# Clases auxiliares del proyecto base. NO se modifica.
add_library(base STATIC
    Config.h
    Config.cpp
    Estadisticas.h
    Estadisticas.cpp
    Traza.h
    Traza.cpp
    Vigilante.h
    Vigilante.cpp
    Lanzador.h
    Lanzador.cpp
    Contexto.h
)
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(base PUBLIC Threads::Threads)
