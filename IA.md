# Registro de Uso de Herramientas de Inteligencia Artificial (IA.md)

**Asignatura:** Programación Concurrente y Distribuida (PCD)  
**Equipo 2:** Marcos de Aza, Ming Xu, Ignacio Catalá y Joan Balbastre  
**Caso 2:** Cocina de un restaurante  

---

## Declaración y Criterio de Uso

En cumplimiento de la normativa de la asignatura (apartado 1.3 de la guía docente), en este documento se detalla de manera transparente el uso de asistentes de IA durante las dos semanas de desarrollo del proyecto.

El equipo utilizó herramientas de IA principalmente como **asistente de depuración, análisis de concurrencia y validación de casos límite** sobre código borrador desarrollado por los miembros, así como para consultar alternativas de balanceo de carga en la competición. Todo el código generado o sugerido fue analizado críticamente, adaptado a las directrices de C++20 y filtrado según las restricciones de la asignatura (RAII estricto, encapsulamiento y ausencia de librerías externas).

---

## Registro Detallado de Consultas

### Consulta 1: Implementación del monitor Semáforo y timeout seguro
* **Fecha:** 14-nov-2026
* **Miembro:** Marcos de Aza
* **Herramienta:** ChatGPT-4o
* **Prompt exacto:**
  > *"Tengo que implementar una clase Semaforo en C++20 que funcione como monitor con mutex y variable de condicion (sin usar la de la libreria estandar). Necesito acquire(), release() y bool tryAcquireFor(std::chrono::milliseconds). ¿Cómo garantizo que tryAcquireFor no sufra de despertares falsos (spurious wakeups) ni se quede bloqueado si expira el tiempo de espera?"*
* **Respuesta obtenida:**
  La IA propuso una estructura clásica de monitor encapsulando un `std::mutex` y un `std::condition_variable`. Para `tryAcquireFor`, sugirió la sintaxis de `wait_for` recibiendo un predicado lambda `[this] { return _contador > 0; }` que devuelve automáticamente `false` si vence el plazo sin cumplirse la condición.
* **Qué se incorporó:**
  La estructura de `tryAcquireFor` con predicado lambda y la gestión de la reducción del contador en exclusión mutua.
* **Qué se descartó / corrigió y por qué:**
  La IA propuso inicialmente usar `notify_all()` en el método `release()`. El equipo lo descartó y lo sustituyó por `notify_one()`, ya que cada llamada a `release` solo habilita una plaza adicional, evitando así el problema de la estampida (*thundering herd problem*).
* **Ficheros y commits:** `semaforo/Semaforo.h`, `semaforo/Semaforo.cpp` (Commit `b5f6186`).

---

### Consulta 2: Balanceo de carga en la competición sin contención de cerrojos
* **Fecha:** 16-nov-2026
* **Miembro:** Ming Xu
* **Herramienta:** Claude 3.5 Sonnet
* **Prompt exacto:**
  > *"Tengo un bucle que calcula el producto modular de 2 millones de llamadas a una función valor(i). El coste computacional de valor(i) no es uniforme y crece conforme i aumenta. Si tengo que paralelizarlo con std::thread entre N hilos, ¿cuál es la estrategia más eficiente para balancear la carga evitando condiciones de carrera y sin usar cerrojos?"*
* **Respuesta obtenida:**
  La IA analizó tres estrategias: reparto por bloques continuos, cola de tareas compartida (*work stealing*) y reparto entrelazado (*stride / round-robin*: cada hilo procesa `h, h+N, h+2N...`). Explicó que el reparto entrelazado distribuye homogéneamente el coste creciente de `valor(i)` entre todos los núcleos sin requerir ninguna sincronización intermedia.
* **Qué se incorporó:**
  El bucle entrelazado con paso `i += hilos` y un vector preasignado `std::vector<std::uint64_t> parciales(hilos, 1)` donde cada hilo escribe únicamente en su índice disjunto.
* **Qué se descartó / corrigió y por qué:**
  La IA sugirió una cola atómica compartida (`std::atomic<int> indice`) para reparto dinámico. Tras medir el rendimiento en pruebas preliminares, se descartó porque la contención en el bus de memoria degradaba los tiempos frente al reparto estático entrelazado (~417 ms frente a ~650 ms).
* **Ficheros y commits:** `competicion/Producto.cpp` (Commit `2e05ed3`).

---

### Consulta 3: Forzado controlado de la condición de carrera en el horno
* **Fecha:** 17-nov-2026
* **Miembro:** Ignacio Catalá (Nacho)
* **Herramienta:** ChatGPT-4o
* **Prompt exacto:**
  > *"En el caso de la cocina, el horno tiene plazas_horno limitadas (3 plazas). Para el modo de fallo --fallo carrera, el enunciado exige que el cocinero compruebe si hay plaza en una sección crítica y la ocupe en otra distinta. ¿Cómo estructuramos esa rotura de atomicidad para garantizar que la carrera ocurra consistentemente en pruebas de estrés sin provocar un cuelgue del programa?"*
* **Respuesta obtenida:**
  La IA sugirió separar la comprobación y la reserva en dos bloques independientes con su propio `std::lock_guard`. En medio de ambos bloques, propuso introducir un pequeño retardo simulado (`std::this_thread::sleep_for(std::chrono::milliseconds(1))`) para crear una ventana de oportunidad donde otros hilos concurrentes también vean plaza libre e intenten entrar simultáneamente.
* **Qué se incorporó:**
  El bucle de sondeo no atómico en `Cocina::entrarHorno()` bajo el flag `_falloCarrera`, que provoca que en `--estres 10` se supere sistemáticamente el límite de plazas y se registre la violación correspondiente.
* **Qué se descartó / corrigió y por qué:**
  La IA propuso eliminar completamente los mutexes de la clase `Cocina` para el modo carrera. Se descartó de inmediato porque el enunciado especifica romper únicamente la atomicidad de la entrada al horno, no provocar una corrupción general de memoria.
* **Ficheros y commits:** `caso/Cocina.cpp` (Commits `8d45dfc` y `f32b453`).

---

### Consulta 4: Equidad y orden FIFO estricto en el monitor del horno
* **Fecha:** 18-nov-2026
* **Miembro:** Ignacio Catalá (Nacho)
* **Herramienta:** Claude 3.5 Sonnet
* **Prompt exacto:**
  > *"Para la regla 3 del horno, los cocineros deben entrar en estricto orden de llegada (FIFO) y el jefe de cocina debe tener prioridad de limpieza (mientras espera para limpiar, ningún plato nuevo puede entrar). ¿Cuál es la forma canónica de implementar esta equidad FIFO dentro de un monitor en C++?"*
* **Respuesta obtenida:**
  La IA recomendó el algoritmo de tickets (*ticket lock* adaptado a monitores): dos contadores enteros `_siguienteTicket` y `_ticketAtendido`. Al llegar, cada cocinero toma su turno (`miTicket = _siguienteTicket++`). En el `cv.wait()`, la condición exige `miTicket == _ticketAtendido && !_limpiando && _enHorno < _plazasHorno`.
* **Qué se incorporó:**
  El sistema de tickets en `Cocina.h` y la condición compuesta del predicado en `entrarHorno()`.
* **Qué se descartó / corrigió y por qué:**
  La IA propuso utilizar una cola de variables de condición (`std::queue<std::condition_variable*>`). Se descartó para mantener una única variable de condición `_cvHorno` compartida con `notify_all()`, lo cual simplifica enormemente la gestión de memoria y la legibilidad del código.
* **Ficheros y commits:** `caso/Cocina.h`, `caso/Cocina.cpp` (Commit `8d45dfc`).

---

### Consulta 5: Cálculo preciso de ocupación media sin alterar la exclusión mutua
* **Fecha:** 20-nov-2026
* **Miembro:** Marcos de Aza
* **Herramienta:** ChatGPT-4o
* **Prompt exacto:**
  > *"El enunciado pide medir la ocupación media de cada buffer calculando la media del número de platos justo después de cada inserción. ¿Cómo lo registramos de forma thread-safe en BufferSemaforos y BufferMonitor sin añadir sobrecoste ni bloqueos extra?"*
* **Respuesta obtenida:**
  La IA indicó que la lectura de `_cola.size()` y la acumulación `_sumaOcupacion += _cola.size()` debe ocurrir inmediatamente después de `_cola.push()` dentro de la misma sección crítica protegida por el mutex que ya posee el hilo, sin necesidad de cerrojos adicionales.
* **Qué se incorporó:**
  Los acumuladores `_sumaOcupacion` y `_numInserciones` dentro de la sección crítica de `poner()` en `BufferSemaforos.cpp` y `BufferMonitor.cpp`.
* **Qué se descartó / corrigió y por qué:**
  Se descartó una propuesta de la IA de calcular la media en punto flotante en cada inserción; en su lugar, se acumulan enteros (`long`) y la división `double` solo se computa al final en el getter `ocupacionMedia()`, optimizando el rendimiento.
* **Ficheros y commits:** `caso/BufferSemaforos.cpp` (Commit `bf94742`), `caso/BufferMonitor.cpp` (Commit `7573bb9`).

---

### Consulta 6: Protocolo de cierre en cascada e invariantes finales
* **Fecha:** 22-nov-2026
* **Miembro:** Joan Balbastre
* **Herramienta:** Claude 3.5 Sonnet
* **Prompt exacto:**
  > *"Tenemos una cadena con camareros, cocineros, emplatadores y un jefe de limpieza periódico. ¿Cuál es el orden riguroso para apagar todos los hilos al finalizar las comandas sin que ningún hilo quede bloqueado esperando en un buffer vacío o en un join infinito?"*
* **Respuesta obtenida:**
  La IA esquematizó el protocolo de cierre en 5 pasos secuenciales:
  1. Esperar al `join()` de todos los camareros.
  2. Introducir tantos valores centinela (`FIN_PLATO = -1`) en el Buffer 1 como cocineros haya (10).
  3. Esperar al `join()` de los 10 cocineros.
  4. Marcar la bandera `_cerrado = true` en el Buffer 2 y despertar a los emplatadores con `notify_all()`.
  5. Esperar al `join()` de los emplatadores y finalmente señalizar la parada del jefe de cocina.
* **Qué se incorporó:**
  La secuencia exacta del protocolo de cierre y la posterior verificación de los invariantes finales en `caso/main.cpp`.
* **Qué se descartó / corrigió y por qué:**
  La IA propuso inicialmente usar variables `std::atomic<bool>` independientes para cada etapa; se descartó para respetar el mecanismo del caso: usar platos de fin en el Buffer 1 y bandera encapsulada dentro del monitor en el Buffer 2.
* **Ficheros y commits:** `caso/main.cpp` (Commits `a8f02fc` y posteriores).

---

## Resumen de Conclusiones

El uso de herramientas de IA permitió:
1. **Verificar la cohesión arquitectónica** entre los módulos diseñados por los distintos miembros del equipo.
2. **Depurar condiciones de carrera sutiles** y contrastar que los modos de fallo requeridos cumplieran estrictamente la semántica del enunciado.
3. **Optimizar la estrategia de partición de carga** en la competición antes de ejecutar las baterías de pruebas definitivas.

Todas las decisiones de sincronización, la elección de los tipos de cerrojos (RAII), el diseño de los predicados y la resolución de las pruebas orales fueron asimiladas, comprendidas y defendidas por los cuatro integrantes del equipo.
