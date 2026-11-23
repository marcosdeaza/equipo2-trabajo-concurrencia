# Ejemplo de uso de las clases auxiliares (no forma parte de la entrega).
add_executable(ejemplo
    main.cpp
    Contador.h
    Contador.cpp
)
target_link_libraries(ejemplo PRIVATE base)
