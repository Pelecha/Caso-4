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

### Resultados medidos (Hito 2)

| Métrica | `--capacidad_tramo 1` | `--capacidad_tramo 4` |
|---|---|---|
| Espera media tren (tramo) | 3.37 ms (máx 17.66 ms) | 1.15 ms (máx 9.82 ms) |
| Espera media tren (andén) | 1.43 ms (máx 13.87 ms) | 0.31 ms (máx 7.35 ms) |
| Espera media mantenimiento | 1.47 ms (máx 3.77 ms) | 1.70 ms (máx 3.53 ms) |
| Pasajeros generados | 1240 | 789 |
| Pasajeros transportados | 1180 | 752 |
| Esperando al final | 60 | 37 |
| Trabajos de mantenimiento | 8 | 5 |
| Invariantes | OK | OK |

### Conclusiones
Se cumple lo que esperábamos en el Hito 1. Con capacidad 4 pueden ir varios trenes a la vez en el mismo sentido, así que la espera media en tramo baja de 3.37 ms a 1.15 ms y en andén de 1.43 ms a 0.31 ms. Al terminar los 300 viajes bastante antes, da tiempo a que se generen menos pasajeros (789 frente a 1240) y el mantenimiento entra menos veces (5 frente a 8). Su espera media sube un poco (1.70 ms frente a 1.47 ms) porque a veces pilla más trenes dentro del tramo antes de que se vacíe.

---

## 2. Pruebas de estrés y modos de fallo (`--estres 10`)

- Ejecución normal (`--estres 10`): 10 de 10 OK, sin invariantes rotos.
- Modo `carrera`: falla en las 10 repeticiones (`INVARIANTE ROTO -> trenes en sentidos opuestos` en los tramos 1, 2 y 3) porque al comprobar y entrar en bloqueos distintos se cuelan trenes de frente.
- Modo `interbloqueo`: en la primera ejecución ya salta el `Vigilante` a los 2000 ms (tras 17 viajes) y corta el programa al quedarse los trenes bloqueados pidiendo andén desde dentro del tramo.
- Modo `inanicion` (con carga alta): falla las 10 veces (`INVARIANTE ROTO -> tramo 0: mantenimiento sufre inanicion esperando tramo vacio`), con esperas de entre 53 ms y 1013 ms sin poder entrar.

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
