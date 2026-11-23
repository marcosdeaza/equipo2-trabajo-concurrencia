cmake_minimum_required(VERSION 3.16)
project(equipo2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(base)
add_subdirectory(semaforo)
add_subdirectory(prueba_semaforo)
add_subdirectory(caso)
add_subdirectory(competicion)
add_subdirectory(ejemplo)
