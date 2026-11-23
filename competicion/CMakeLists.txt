# Competición (apartado 6). Solo se modifica Producto.cpp.
add_executable(competicion
    main.cpp
    Valor.h
    Valor.cpp
    Producto.h
    Producto.cpp
)
target_link_libraries(competicion PRIVATE Threads::Threads)
