# Registro de Consultas y Dudas Conceptuales (IA.md)

De acuerdo con las directrices del curso para el registro de herramientas de apoyo al estudio, en este documento se recopilan las consultas conceptuales y dudas de sintaxis realizadas durante el desarrollo del trabajo.

---

### Consulta 1: Diferencia de comportamiento en variables de condición
- **Fecha:** 14-nov-2026
- **Miembro:** Nacho Catalá
- **Herramienta:** Asistente web
- **Consulta realizada:** "¿Por qué es obligatorio usar un predicado dentro de `cv.wait()` en vez de un `if`?"
- **Respuesta y conclusión:** Se repasó el concepto de despertares falsos (*spurious wakeups*), confirmando que el hilo debe volver a evaluar la condición al despertar antes de continuar.
- **Aplicación:** Se añadieron predicados en forma de lambda a todas las llamadas `wait()` de la clase `BufferMonitor` y el horno.

---

### Consulta 2: Sintaxis de temporizadores en C++20
- **Fecha:** 16-nov-2026
- **Miembro:** Ming Xu
- **Herramienta:** Asistente web
- **Consulta realizada:** "¿Cómo se calcula la diferencia de tiempo entre dos puntos usando `std::chrono::steady_clock`?"
- **Respuesta y conclusión:** `auto duracion = fin - inicio;` y paso directo a las funciones de estadísticas.
- **Aplicación:** Medición de tiempos de espera en `Cocina.cpp`.

---

### Consulta 3: Prevención de interbloqueo circular
- **Fecha:** 18-nov-2026
- **Miembro:** Marcos de Aza
- **Herramienta:** Asistente web
- **Consulta realizada:** "En el problema de los filósofos con recursos numerados en círculo, ¿por qué asignar un orden global menor-mayor elimina el ciclo?"
- **Respuesta y conclusión:** Confirma que al forzar `std::min` y `std::max`, el último nodo rompe la simetría y elimina la espera circular.
- **Aplicación:** Lógica de toma de utensilios en `Cocina::cocinero`.

---

### Nota del Equipo
Todo el código fuente, la arquitectura de las clases, las tablas de predicados y la resolución de la simulación han sido desarrollados, razonados y verificados directamente por los miembros del Equipo 2.
