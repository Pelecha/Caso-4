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

- **Normal (`./ferrocarril --estres 10`):** pendiente
- **Carrera (`./ferrocarril --fallo carrera --estres 10`):** pendiente
- **Interbloqueo (`./ferrocarril --fallo interbloqueo --estres 10`):** pendiente
- **Inanición (`./ferrocarril --fallo inanicion --estaciones 2 --andenes 30 --trenes 40 --capacidad_tramo 10 --t_tramo 10 --viajes 20 --estres 10`):** pendiente

**Por qué a veces falla y a veces no:**
Pendiente de redactar con las trazas del Hito 2.

---

## 3. Competición (`competicion/Producto.cpp`) - Hito 2

- **Mediana de 5 ejecuciones (Release):**
  - 1 hilo: -
  - 2 hilos: -
  - 4 hilos: -
  - 8 hilos: -

**Análisis:**
Pendiente de medir en el Hito 2.

---

## 4. Notas de configuración (`base/`)

Claves registradas en `Lanzador` para `caso4`:
- `estaciones` (5), `andenes` (2), `trenes` (10), `capacidad_tramo` (2), `viajes` (30), `t_tramo` (3).
- Modos de fallo: `carrera`, `interbloqueo`, `inanicion`.
- `vigilante.progreso()` se llama al completar cada viaje y `estadisticas` registra las esperas y violaciones de invariante.
