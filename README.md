# Trabajo por equipos - Equipo 2

Programación Concurrente y Distribuida (Temas 3 a 8).

**Equipo:** Marcos, Ming Xu, Mireia, Nacho y Joan.

**Caso:** 2 - Cocina de un restaurante.

## Estructura

- `base/`: clases auxiliares del profesor (no se tocan).
- `semaforo/`: nuestra clase `Semaforo`.
- `prueba_semaforo/`: prueba de `Semaforo`.
- `caso/`: nuestro caso (la cocina). Hecho el modo normal y el modo `carrera`.
- `competicion/`: programa de la competición.
- `ejemplo/`: ejemplo del profesor, solo para mirar.
- `docs/proyecto_base.md`: README original del profesor.

## Cómo compilar

```bash
cmake -S . -B build
cmake --build build
```

## Cómo ejecutar la prueba de Semaforo

```bash
./build/prueba_semaforo/prueba_semaforo
```

Tiene que terminar con `OK`.

## Cómo ejecutar el caso

```bash
./build/caso/caso                  # version correcta, tiene que dar OK
./build/caso/caso --fallo carrera  # rompe el invariante del horno
```

## Estado del hito 1

- [x] Fase 1: proyecto base compilando
- [x] Fase 2: `Semaforo` y `prueba_semaforo` imprime OK
- [x] Fase 3: caso 2 con hilos y modo `carrera`
- [ ] Fase 4: `EXPERIMENTOS.md` con la predicción
- [ ] Fase 5: etiqueta `hito1`
