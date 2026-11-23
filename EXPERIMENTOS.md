# Experimentos - Caso 2 (cocina de un restaurante)

## Experimento: --cola 1, 4 y 32

Pregunta: que etapa es el cuello de botella? Cambia algo el tamano de la cola de los buffers?

### Prediccion (escrita antes de ejecutar nada)

Cuentas por etapa, con la configuracion por defecto:

- **Camareros**: no gastan tiempo. Solo llenan el buffer 1 todo lo rapido que les dejan.
- **Cocineros**: cada plato tarda al menos 1 + 2 ms con los utensilios y 3 ms en el horno, es decir, unos 6 ms. Con 10 cocineros podrian sacar unos 1,6 platos por ms, asi que no son el limite.
- **Horno**: 3 plazas y 3 ms por plato, o sea como mucho 1 plato por ms. Ademas, cada 20 ms el jefe lo limpia durante 5 ms y nadie entra en ese tiempo. Eso deja unos 0,7 platos por ms de capacidad real.
- **Emplatadores**: 2 emplatadores y 1 ms por plato, unos 2 platos por ms. No son el limite.

Por tanto, creo que el **cuello de botella es el horno**. Los 120 platos deberian tardar unos 170 ms de trabajo util, mas el tiempo de dormir y de arrancar hilos.

Lo que espero ver:

| Cola | Tiempo total (aprox.) | Ocupacion media buffer 1 | Ocupacion media buffer 2 |
|---|---|---|---|
| 1 | parecido a los demas | cerca de 1 | cerca de 0 |
| 4 | parecido a los demas | cerca de 4 (casi siempre llena) | cerca de 0 |
| 32 | parecido a los demas | entre 20 y 32 | cerca de 0 |

Razonamiento: los camareros llenan el buffer 1 mucho mas rapido de lo que el horno lo vacia, asi que el buffer 1 se llena y los platos esperan delante del horno. El buffer 2 casi no se usa, porque los emplatadores son mas rapidos que el horno.

Con cola 1 el atasco no se ve en el buffer 1, porque el plato espera en la mano del cocinero. Por eso creo que el tiempo total no cambia mucho con la cola: lo que cambia es donde se acumulan los platos.

Lo que me sorprenderia: que el buffer 2 se llene, o que el tiempo total cambie mucho entre colas.

### Resultado real

Se ejecutan las tres configuraciones con las métricas de ocupación media implementadas:

```bash
./build/caso/caso --cola 1
./build/caso/caso --cola 4
./build/caso/caso --cola 32
```

#### Tabla de Mediciones

| Cola | Tiempo total (aprox.) | Ocupación media buffer 1 (x100) | Ocupación media buffer 2 (x100) | Observaciones |
|---|---|---|---|---|
| **1** | ~230 ms | 100 (media: **1.00**) | 100 (media: **1.00**) | Ambos buffers saturados a su máxima capacidad (1) |
| **4** | ~225 ms | 359 (media: **3.59**) | 133 (media: **1.33**) | Buffer 1 casi siempre lleno (3.59/4); Buffer 2 casi vacío |
| **32** | ~220 ms | 2793 (media: **27.93**) | 131 (media: **1.31**) | Acumulación masiva en Buffer 1 (28 platos); Buffer 2 fluye rápido |

#### Conclusión: ¿Cuál es el cuello de botella?

El **horno es el auténtico cuello de botella de la cocina**.
- Los 4 camareros generan comandas a gran velocidad y saturan de inmediato el Buffer 1 (la ocupación media escala hasta casi 28 platos cuando la capacidad lo permite).
- Sin embargo, el Buffer 2 mantiene siempre una ocupación muy baja (~1.3 platos en media), independientemente del tamaño de la cola, debido a que los 2 emplatadores procesan cada plato en 1 ms, siendo sustancialmente más veloces que la tasa de salida del horno (3 ms por tanda de 3 plazas, más 5 ms de parada cada 20 ms por limpieza).
- Variar el tamaño del buffer no acelera sensiblemente la producción total: solo determina dónde se acumula la espera física (en el Buffer 1 o retenida por los camareros esperando hueco).

---

## Modos de Fallo y Pruebas de Estrés

A continuación se adjuntan las salidas reales obtenidas en terminal para cada modo:

### 1. Versión Correcta (`--estres 10`)
```text
$ ./build/caso/caso --estres 10
caso (versión correcta)

--estres 10: 10 OK, 0 con invariantes rotos (misma configuración en todas).
```

### 2. Modo Carrera (`--fallo carrera --estres 10`)
```text
$ ./build/caso/caso --fallo carrera --estres 10
caso --fallo carrera
  ejecución 1 de 10: INVARIANTE ROTO -> un plato ha entrado al horno mientras el jefe limpiaba
  ejecución 2 de 10: INVARIANTE ROTO -> más platos en el horno (4) que plazas (3)
  ejecución 3 de 10: INVARIANTE ROTO -> más platos en el horno (4) que plazas (3)
  ejecución 4 de 10: INVARIANTE ROTO -> un plato ha entrado al horno mientras el jefe limpiaba
  ejecución 5 de 10: INVARIANTE ROTO -> más platos en el horno (4) que plazas (3)
  ...
--estres 10: 0 OK, 10 con invariantes rotos (misma configuración en todas).
```

### 3. Modo Interbloqueo (`--fallo interbloqueo`)
```text
$ ./build/caso/caso --fallo interbloqueo
caso --fallo interbloqueo

*** VIGILANTE (ejecución única): sin progreso durante 2000 ms. Posible interbloqueo, livelock o despertar perdido. ***

=== Esperas por tipo de hilo ===
  camarero (buffer 1 lleno)                    n=    14  media=    0.04 ms  max=    0.07 ms
  jefe (horno vacío)                          n=    71  media=    0.00 ms  max=    0.00 ms
=== Métricas ===
  limpiezas                                    71
  platos pedidos                               14
=== Invariantes ===
  OK
Se termina el proceso: los hilos bloqueados no se pueden despertar.
# Código de salida: 3
```

### 4. Modo Inanición (`--fallo inanicion`)
```text
$ ./build/caso/caso --fallo inanicion
caso --fallo inanicion

=== Esperas por tipo de hilo ===
  camarero (buffer 1 lleno)                    n=   120  media=    4.45 ms  max=   14.68 ms
  cocinero (horno)                             n=   120  media=    4.15 ms  max=    6.55 ms
  cocinero (utensilios)                        n=   120  media=    1.17 ms  max=   24.71 ms
  emplatador (buffer 2 vacío)                 n=   122  media=    1.38 ms  max=    9.98 ms
  jefe (horno vacío)                          n=     1  media=  133.23 ms  max=  133.23 ms
=== Métricas ===
  limpiezas                                    1
  ocupación media buffer 1 (x100)             390
  ocupación media buffer 2 (x100)             103
  platos emplatados                            120
  platos pedidos                               120
=== Invariantes ===
  OK
```
*Observación*: Mientras que en la versión correcta el jefe limpia 7 veces con una espera media de ~1.6 ms (máxima de ~3.6 ms), en el modo inanición el jefe queda atrapado esperando **133.23 ms** a que el horno quede vacío, pudiendo limpiar únicamente 1 vez al final de toda la simulación.

### ¿Por qué la misma configuración falla unas veces sí y otras no?
En sistemas concurrentes, el orden exacto en que los hilos obtienen la CPU no es determinista, sino que depende del **planificador (scheduler) del sistema operativo**. 
En el modo de carrera, para que el invariante se rompa es necesario que dos o más hilos ejecuten la comprobación de plazas disponibles simultáneamente antes de que cualquiera de ellos ejecute el incremento. Si el planificador concede un cuanto de tiempo suficiente a un hilo para comprobar e incrementar sin interrupción, esa iteración particular no manifestará el fallo. Por este motivo, la ausencia de fallos en una sola ejecución no demuestra la corrección del programa, siendo indispensable el test de estrés (`--estres`).

---

## Competición de Rendimiento (`competicion/Producto.cpp`)

Mediciones reales obtenidas en una máquina de desarrollo con procesador Apple Silicon (10 núcleos de CPU):

Comando de prueba:
```bash
./build/competicion/competicion --hilos H --veces 5
```

### Resultados de Mediana por Hilos

| Hilos | Tiempo Mediana | Aceleración (Speedup) |
|---|---|---|
| **1 hilo** | 2471.57 ms | 1.00x (referencia secuencial) |
| **2 hilos** | 1258.10 ms | 1.96x (casi óptimo) |
| **4 hilos** | 694.11 ms | 3.56x |
| **8 hilos** | 409.19 ms | 6.04x |

### Explicación de los Tiempos
El cálculo de `valor(i)` presenta una carga desbalanceada (requiere más multiplicaciones a medida que $i$ crece hacia 2.000.000). Para evitar que los hilos asignados a los bloques finales tarden mucho más que los iniciales, se implementó un **reparto entrelazado (round-robin / stride)** donde el hilo $h$ computa $h, h+\text{hilos}, h+2\cdot\text{hilos}\dots$
Esto distribuye la carga pesada de forma perfectamente homogénea entre todos los hilos, alcanzando una reducción de tiempo desde 2.47 s a **0.40 s** con 8 hilos.

La versión para la competición del equipo se configurará para medirse con **8 hilos** (especificado en `README.md`).

