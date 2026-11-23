# Diseño de Sincronización — Caso 2 (Cocina de un restaurante)

**Equipo 2:** Marcos de Aza, Ming Xu, Ignacio Catalá, Joan Balbastre, Mireia Martínez.  
**Asignatura:** Programación Concurrente y Distribuida.  

---

## 1. Tabla de Predicados de los Monitores (Formato Tema 7, Ejercicio 4)

Para cada operación de sincronización basada en variables de condición, se describe formalmente la condición de guarda (*espera mientras...*) y el responsable del desbloqueo (*quién avisa*).

| Monitor / Componente | Operación | Espera mientras... (Condición de bloqueo) | Quién avisa (Notificación) |
|---|---|---|---|
| **`Semaforo`** | `acquire()` | `_valor == 0` (no hay plazas disponibles) | `release()` con `_cv.notify_one()` al devolver una plaza |
| **`Semaforo`** | `tryAcquireFor(p)` | `_valor == 0` hasta que venza el plazo `p` | `release()` con `_cv.notify_one()` |
| **`BufferMonitor`** (Buffer 2) | `poner(plato)` | `_cola.size() >= _capacidad` (buffer lleno) | `tomar()` con `_hayHueco.notify_one()` al sacar un plato |
| **`BufferMonitor`** (Buffer 2) | `tomar(plato)` | `_cola.empty() && !_cerrado` (buffer vacío y abierto) | `poner()` con `_hayPlato.notify_one()` al insertar, o `cerrar()` con `_hayPlato.notify_all()` |
| **`Horno`** (en `Cocina`) | `entrarHorno()` | `miTicket != _ticketAtendido \|\| _limpiando \|\| _enHorno >= _plazasHorno` | `salirHorno()` (libera plaza) o `jefe()` al terminar de limpiar, ambos con `_cvHorno.notify_all()` |
| **`Jefe de Cocina`** (en `Cocina`) | Limpieza periódica | `_enHorno > 0` (espera a que el horno esté totalmente vacío) | `salirHorno()` con `_cvHorno.notify_all()` cuando el último cocinero sale del horno |

---

## 2. Explicación de los Mecanismos de Sincronización

### 2.1 Buffer 1 (`BufferSemaforos` — Regla 1)
El Buffer 1 implementa el patrón clásico del productor-consumidor delimitado mediante semáforos contadores:
- **`_huecos` (`Semaforo` propio)**: Inicializado a `capacidad`. Representa las plazas libres. Los camareros hacen `_huecos.acquire()` antes de insertar.
- **`_platos` (`std::counting_semaphore`)**: Inicializado a 0. Representa los platos cocinados disponibles para recoger. Los cocineros hacen `_platos.acquire()` antes de extraer.
- **`_m` (`std::mutex`)**: Exclusión mutua estricta para proteger la estructura de datos interna `std::queue<int> _cola`. El cerrojo solo se retiene durante el `push()` o `pop()` en memoria (fracciones de microsegundo), nunca durante las esperas.

### 2.2 Utensilios de Cocina y Prevención de Interbloqueo (Regla 2, Tema 8)
Existen 10 cocineros y 10 utensilios distribuidos en círculo. Cada cocinero $i$ requiere su utensilio izquierdo $i$ y el derecho $(i+1) \pmod{10}$.
- **El problema (Espera circular)**: Si cada cocinero toma primero su utensilio izquierdo, espera 1 ms y luego intenta tomar el derecho, los 10 cocineros retendrán un utensilio mientras esperan eternamente por el del vecino, generando un **interbloqueo (deadlock)**.
- **La solución (Orden total)**: Se asigna un orden global a los recursos. Todo cocinero adquiere obligatoriamente primero el utensilio de **índice numérico menor** y luego el de **índice mayor**:
  $$\text{primero} = \min(i, (i+1) \pmod{10}), \quad \text{segundo} = \max(i, (i+1) \pmod{10})$$
  Esto rompe de raíz la cuarta condición de Coffman (**espera circular**). El cocinero 9, en lugar de solicitar 9 y luego 0, solicitará 0 primero y luego 9, destruyendo la simetría y garantizando que al menos un cocinero siempre pueda progresar.

### 2.3 Horno FIFO y Prevención de Inanición (Regla 3)
El horno es un monitor con capacidad máxima de `plazas_horno` (3 por defecto). Debe cumplir dos requisitos críticos:
1. **Orden de llegada estricto (FIFO)**: Se implementa un protocolo de tickets (tipo panadería):
   - Al invocar `entrarHorno()`, el cocinero toma un ticket atómico secuencial: `miTicket = _siguienteTicket++`.
   - La condición de espera comprueba `miTicket == _ticketAtendido`. Ningún cocinero posterior puede adelantar al turno actual, aunque haya plazas libres en el horno.
2. **Prioridad y ausencia de inanición para el Jefe de Cocina**:
   - Cada `limpieza` ms (20 ms), el jefe activa la bandera `_limpiando = true` **antes de bloquearse** a esperar que el horno se vacíe.
   - En cuanto `_limpiando` es verdadero, el predicado de `entrarHorno()` se evalúa a falso para cualquier nuevo cocinero.
   - De este modo, los platos que ya están dentro terminan de hornearse (como máximo 3 ms), el horno llega a 0 platos y el jefe limpia inmediatamente (5 ms), evitando que el jefe quede esperando indefinidamente mientras entra un flujo continuo de platos.

### 2.4 Cierre Ordenado de la Cocina (Fase de Terminación)
Para evitar bloqueos o fugas de hilos al terminar la jornada, se sigue una secuencia estricta en `main.cpp`:
1. `join()` de todos los camareros (ya han emitido todas sus comandas).
2. Se insertan exactamente **10 platos especiales `FIN_PLATO` (-1)** en el Buffer 1 (uno para cada cocinero).
3. Cada cocinero, al extraer `FIN_PLATO`, sale de su bucle de trabajo y termina.
4. `join()` de los 10 cocineros.
5. Se invoca `cocina.cerrarSalida()`, que activa `_cerrado = true` en el Buffer 2 y ejecuta `_hayPlato.notify_all()`.
6. Los emplatadores vacían los platos restantes en el Buffer 2; al ver la cola vacía y `_cerrado == true`, finalizan limpiamente.
7. `join()` de los emplatadores.
8. Se invoca `cocina.pararJefe()`, que pone `_parar = true`, y se hace `join()` del jefe de cocina.
9. Se detiene el `Vigilante` y se verifican los invariantes finales.

---

## 3. Decisiones de Diseño

- **Gestión de recursos (RAII)**: Se utiliza `std::lock_guard` para secciones críticas simples y `std::unique_lock` cuando se asocian a variables de condición `std::condition_variable`. No se realizan llamadas explícitas a `.lock()` o `.unlock()`.
- **Medición de esperas**: Se envuelven las llamadas bloqueantes con `std::chrono::steady_clock::now()` antes y después de la suspensión, registrando la duración exacta en `Estadisticas::registrarEspera()`.
- **Cálculo de ocupación media**: Se acumula la longitud de la cola justo tras cada inserción (`_sumaOcupacion += _cola.size()`) bajo el mutex. En `main.cpp` se registra escalada en centésimas (x100) para preservar dos decimales dentro del sistema de enteros de `Estadisticas`.
