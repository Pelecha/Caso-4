# Experimentos - Caso 4 (Línea de ferrocarril)

## 1. Comparativa: `--capacidad_tramo 1` vs `--capacidad_tramo 4`

### Parámetros de partida
Usamos la configuración base del programa:
- 5 estaciones (4 tramos de vía única)
- 2 andenes por sentido en cada estación
- 10 trenes haciendo 30 viajes cada uno
- Tiempo por tramo (`t_tramo`): 3 ms

### Predicción antes de ejecutar (Hito 1)

**Con `--capacidad_tramo 1`:**
Al limitar la capacidad a 1, solo pasa un tren cada vez por cada tramo, aunque vayan en la misma dirección. Los trenes van a hacer mucha cola antes de entrar a los tramos porque cada uno tiene que esperar los 3 ms del anterior. En cambio, en los andenes apenas habrá esperas porque llegan de uno en uno y cada estación tiene 2 andenes libres. El tiempo total será bastante alto al ir todo en fila.

**Con `--capacidad_tramo 4`:**
Aquí pueden meterse hasta 4 trenes a la vez en el mismo tramo si van en el mismo sentido y no toca mantenimiento. La espera para entrar al tramo bajará muchísimo; los trenes solo frenarán cuando venga otro de frente o cuando salte el mantenimiento (5 ms cada 30 ms). Lo que sí subirá un poco es la espera por andén: como entran en grupo al tramo, llegan varios de golpe a la estación y, al haber solo 2 andenes por sentido, el tercero tendrá que esperar antes de entrar al tramo a que se vacíe uno. En general terminará bastante antes que con capacidad 1, aunque no cuatro veces más rápido por los cambios de sentido y el límite de andenes.

### Resultados obtenidos (Hito 2)

| Métrica | `--capacidad_tramo 1` | `--capacidad_tramo 4` |
| :--- | :--- | :--- |
| Tiempo total | 1452 ms | 1259 ms |
| Espera tren (tramo) | media: 10.73 ms (max: 79.12 ms) | media: 5.35 ms (max: 47.23 ms) |
| Espera tren (andén) | media: 3.08 ms (max: 47.30 ms) | media: 1.79 ms (max: 47.25 ms) |
| Espera mantenimiento | media: 3.34 ms (max: 16.06 ms) | media: 6.15 ms (max: 16.06 ms) |

**Conclusiones tras medir:**
- Al aumentar la capacidad de tramo de 1 a 4, la espera media de los trenes en los tramos se reduce a la mitad (pasa de 10.73 ms a 5.35 ms), lo que hace que el tiempo total de la simulación disminuya de 1452 ms a 1259 ms.
- En cambio, la espera de mantenimiento aumenta (pasa de 3.34 ms a 6.15 ms de media). Esto ocurre porque al haber más trenes circulando simultáneamente en un mismo tramo, cuando el mantenimiento intenta cerrar la vía tiene que esperar más tiempo a que salgan todos los trenes que estaban dentro hasta que el contador queda a 0.
- El tiempo global no se reduce cuatro veces porque la vía única sigue exigiendo alternancia de sentidos (no pueden cruzarse trenes en sentido opuesto) y porque los andenes de las estaciones actúan como cuello de botella cuando varios trenes llegan en convoy.

---

## 2. Pruebas de fallos (`--estres 10`) - Hito 2

- **Normal (`./ferrocarril --estres 10`):**
```text
ferrocarril (versión correcta)

--estres 10: 10 OK, 0 con invariantes rotos (misma configuración en todas).
```
Todas las iteraciones pasan sin errores ni violaciones.

- **Carrera (`./ferrocarril --fallo carrera --estres 10`):**
```text
ferrocarril --fallo carrera
  ejecución 1 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 2 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 3 de 10: INVARIANTE ROTO -> tramo 1: trenes en sentidos opuestos
  ejecución 4 de 10: INVARIANTE ROTO -> tramo 1: trenes en sentidos opuestos
  ejecución 5 de 10: INVARIANTE ROTO -> tramo 3: trenes en sentidos opuestos
  ejecución 6 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 7 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 8 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 9 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos
  ejecución 10 de 10: INVARIANTE ROTO -> tramo 2: trenes en sentidos opuestos

--estres 10: 0 OK, 10 con invariantes rotos (misma configuración en todas).
```

- **Interbloqueo (`./ferrocarril --fallo interbloqueo --estres 10`):**
```text
ferrocarril --fallo interbloqueo

*** VIGILANTE (ejecución 1 de 10): sin progreso durante 2000 ms. Posible interbloqueo, livelock o despertar perdido. ***

=== Esperas por tipo de hilo ===
  mantenimiento (tramo vacío)                 n=     4  media=   15.76 ms  max=   16.04 ms
  tren (andén)                                n=    49  media=    1.26 ms  max=   15.98 ms
  tren (tramo)                                 n=    43  media=   10.84 ms  max=   92.95 ms
=== Métricas ===
  pasajeros generados                          227
  trabajos de mantenimiento                    4
  transportados                                56
  viajes                                       39
=== Invariantes ===
  OK
Se termina el proceso: los hilos bloqueados no se pueden despertar.
```

- **Inanición (`./ferrocarril --fallo inanicion --estaciones 2 --andenes 30 --trenes 40 --capacidad_tramo 10 --t_tramo 10 --viajes 20 --estres 10`):**
```text
ferrocarril --fallo inanicion
  ejecución 1 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (360 ms)
  ejecución 2 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (387 ms)
  ejecución 3 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (1120 ms)
  ejecución 4 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (762 ms)
  ejecución 5 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (216 ms)
  ejecución 6 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (983 ms)
  ejecución 7 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (967 ms)
  ejecución 8 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (1292 ms)
  ejecución 9 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (466 ms)
  ejecución 10 de 10: INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio (1305 ms)

--estres 10: 0 OK, 10 con invariantes rotos (misma configuración en todas).
```

**Por qué el planificador hace que falle de manera distinta en cada ejecución:**
- El planificador del sistema operativo reparte los tiempos de CPU (cuantos de tiempo) de forma no determinista entre todos los hilos concurrentes. Por tanto, el entrelazado de instrucciones nunca es idéntico entre dos ejecuciones sucesivas.
- En el modo **carrera**, el fallo se fuerza soltando el cerrojo entre la comprobación de vía libre y la actualización de los contadores (`std::this_thread::yield()`). Según el orden en el que el planificador despache los trenes listos, en unas ejecuciones la colisión ocurre en el tramo 1, en otras en el tramo 2 y en otras en el tramo 3.
- En el modo **interbloqueo**, los trenes entran al tramo antes de reservar el andén de destino. Si dos trenes en sentidos opuestos entran a sus respectivos tramos y ambos intentan reservar andenes ya saturados por el otro sentido, se produce un ciclo de espera (deadlock). El momento en el que coincide esta combinación varía según la velocidad a la que el planificador avance cada tren; en cuanto se produce, ningún hilo avanza y el `Vigilante` detecta falta de progreso tras 2000 ms y aborta.
- En el modo **inanición**, los trenes ignoran que el mantenimiento está solicitando el tramo. Con 40 trenes y alta carga, el tramo casi nunca queda a 0. Dependiendo de los pequeños desfases temporales que introduzca el planificador al suspender y reanudar hilos, el mantenimiento sufre esperas muy dispares en cada ejecución (oscilando entre 216 ms y más de 1300 ms).

---

## 3. Competición (`competicion/Producto.cpp`) - Hito 2

- **Mediana de 5 ejecuciones (Release):**
  - 1 hilo: 5607.93 ms
  - 2 hilos: 4241.29 ms
  - 4 hilos: 2479.73 ms
  - 8 hilos: 1332.07 ms

**Análisis:**
- El cálculo consiste en calcular el producto de 2.000.000 de valores módulo 1000000007. Es un problema estrictamente dependiente de CPU.
- Nuestra implementación reparte el rango $[0, 2000000)$ en bloques contiguos de igual tamaño para cada hilo. Cada hilo calcula su producto parcial de forma completamente independiente sobre una variable local, por lo que no hay sincronización intermedia ni bloqueo por mutex ni contención en memoria caché durante el grueso del cómputo.
- Al final, el hilo principal une los resultados parciales con solo $hilos - 1$ multiplicaciones modulares.
- Con 1 hilo tarda 5607.93 ms. Con 2 hilos baja a 4241.29 ms. Con 4 hilos baja a 2479.73 ms (speedup de 2.26x), y con 8 hilos se reduce a 1332.07 ms (speedup de 4.21x). La mejora de rendimiento es notable a medida que se aprovechan los núcleos lógicos del procesador.

---

## 4. Notas de configuración (`base/`)

Claves registradas en `Lanzador` para `caso4`:
- `estaciones` (5), `andenes` (2), `trenes` (10), `capacidad_tramo` (2), `viajes` (30), `t_tramo` (3).
- Modos de fallo: `carrera`, `interbloqueo`, `inanicion`.
- `vigilante.progreso()` se llama al completar cada viaje y `estadisticas` registra las esperas y violaciones de invariante.
