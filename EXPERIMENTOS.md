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

Pendiente. Se completara en otro commit, despues de ejecutar:

```bash
./build/caso/caso --cola 1
./build/caso/caso --cola 4
./build/caso/caso --cola 32
```

Nota: la ocupacion media de cada buffer todavia no esta medida en el codigo. Hay que anadirla (media del numero de platos justo despues de cada insercion) antes de poder comprobar la tabla.
