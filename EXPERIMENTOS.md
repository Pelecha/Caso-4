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

### Resultados reales

| Métrica | `--capacidad_tramo 1` | `--capacidad_tramo 4` |
|---|---|---|
| Espera media tren en tramo | 3.37 ms, máx 17.66 ms | 1.15 ms, máx 9.82 ms |
| Espera media tren en andén | 1.43 ms, máx 13.87 ms | 0.31 ms, máx 7.35 ms |
| Espera media mantenimiento | 1.47 ms, máx 3.77 ms | 1.70 ms, máx 3.53 ms |
| Pasajeros generados | 1240 | 789 |
| Pasajeros transportados | 1180 | 752 |
| Esperando al final | 60 | 37 |
| Trabajos de mantenimiento | 8 | 5 |
| Invariantes | OK | OK |

Se cumple lo que habíamos previsto en el Hito 1. Al subir la capacidad a 4 pueden pasar varios trenes seguidos en el mismo sentido, así que la espera en tramo baja de 3.37 ms a 1.15 ms y en andén de 1.43 ms a 0.31 ms. Como los 300 viajes se acaban bastante antes, se generan menos pasajeros durante la ejecución y hacen falta menos ciclos de mantenimiento. Lo único que sube un poco es la espera del propio mantenimiento, porque al haber más trenes dentro del tramo a la vez tarda algo más en vaciarse.

---

## 2. Pruebas de estrés y modos de fallo

- Con `--estres 10` normal pasan las 10 ejecuciones seguidas sin romper ningún invariante.
- Con `--fallo carrera` fallan las 10 repeticiones porque entran trenes en sentidos opuestos a la vez en los tramos 1, 2 y 3.
- Con `--fallo interbloqueo` salta el vigilante a los 2000 ms en la primera ejecución tras 17 viajes, ya que los trenes se quedan bloqueados pidiendo andén desde dentro del tramo.
- Con `--fallo inanicion` y carga alta fallan las 10 veces: el mantenimiento se queda esperando en el tramo 0 entre 53 ms y 1013 ms sin llegar a entrar.

**Por qué el planificador hace que falle de manera distinta en cada ejecución:**
- El planificador del sistema operativo reparte los tiempos de CPU de forma no determinista entre todos los hilos concurrentes, por lo que el entrelazado de instrucciones nunca es idéntico entre dos ejecuciones sucesivas.
- En **carrera**, el fallo se fuerza soltando el cerrojo entre comprobar y anotar (`std::this_thread::yield()`); según el hilo que tome la CPU en ese instante, la colisión se manifiesta en el tramo 1, 2 o 3.
- En **interbloqueo**, el momento en el que se cierra el ciclo de espera mutua depende de la velocidad relativa a la que avanzan los trenes en sentidos opuestos; por eso el número de viajes completados antes del bloqueo no es fijo (a veces 17, a veces 39 viajes).
- En **inanición**, las pequeñas pausas de los trenes al subir/bajar pasajeros crean ventanas de tiempo variables, haciendo que la espera del mantenimiento oscile entre unas pocas decenas de milisegundos y más de 1 segundo.

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
