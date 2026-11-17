# Registro de uso de IA

Cada consulta a una herramienta de IA que aparezca en el repositorio se apunta aqui.

## Uso de IA (Equipo 2)

- **Modelo:** Haiku 5.5 (Anthropic)
- **Plataforma:** Claude Code
- **Uso:** Revisión de código, sugerencias de mejora, corrección de errores
- **Contribución:** Ayuda en conceptos de concurrencia, sincronización y depuración
- **Nota:** Todo el código implementado es del equipo 2

## Prompts utilizados

### 1. Predicción de experimento --cola
```
Prompt: Quiero hacer una predicción para una simulación de cocina de restaurante. 
Tengo 10 cocineros, 3 plazas en el horno, 2 emplatadores. 
Predice qué es el cuello de botella y qué ocurrirá con las colas 1, 4 y 32.
```

### 2. Revisión de código sincronización
```
Prompt: Revisa este código de sincronización con semáforos. 
¿Hay algún problema de deadlock o carrera? ¿Crees que el enfoque es correcto?
```

### 3. Depuración errores de compilación
```
Prompt: Tengo un error de compilación en C++ con threads y mutex. 
¿Puedes ayudarme a entender qué está fallando en esta configuración?
```

### 4. Configuración CMake
```
Prompt: Ayúdame a configurar CMakeLists.txt para un proyecto con múltiples directorios.
¿Cómo debo enlazar las librerías estáticas correctamente?
```

### 5. Corrección conceptos
```
Prompt: Estoy confundido sobre el uso de condition variables en C++. 
¿Cuándo uso wait() vs wait_for()? ¿Cómo evito falsos despiertos?
```

### 6. Patrones de diseño
```
Prompt: Para una cola productor-consumidor con límite, ¿qué patrón es mejor?
¿Mutex + condition_variable o algo como std::counting_semaphore?
```

## Miembros del equipo

- Marcos
- Ming Xu
- Mireia
- Nacho
- Joan

**Nota:** Este documento documenta el uso de IA como copiloto, no como generador autónomo del trabajo.
