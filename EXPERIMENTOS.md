# Experimentos — Caso 4: Línea de ferrocarril

## 1. Experimento principal: `--capacidad_tramo 1` frente a `--capacidad_tramo 4`

### Configuración base
Se mantienen el resto de parámetros con sus valores por defecto:
* `--estaciones 5` (4 tramos de vía única: tramos 0, 1, 2 y 3)
* `--andenes 2` (andenes por sentido en cada estación)
* `--trenes 10`
* `--viajes 30`
* `--t_tramo 3` (ms)

### Predicción razonada (Hito 1 — antes de ejecutar la versión sincronizada)

1. **Con `--capacidad_tramo 1` (exclusión mutua estricta por tramo):**
   * **Comportamiento:** Solo puede circular 1 tren a la vez por cada tramo de vía única, independientemente de si los trenes que esperan van en el mismo sentido o en sentido opuesto. No es posible circular en "pelotón".
   * **Espera de trenes (tramo):** **Alta**. Cada tren que llegue a un tramo ocupado tendrá que esperar al menos `t_tramo` (3 ms) a que salga el anterior, acumulándose colas a la entrada de los tramos.
   * **Espera de trenes (andén):** **Baja**. Al salir los trenes escalonados de uno en uno, rara vez coincidirán más de 2 trenes (`--andenes 2`) intentando llegar al mismo sentido de la siguiente estación.
   * **Tiempo total de ejecución:** **Elevado**, al quedar serializado el paso por todos los tramos de la línea.

2. **Con `--capacidad_tramo 4` (concurrencia en el mismo sentido):**
   * **Comportamiento:** Hasta 4 trenes pueden compartir un mismo tramo simultáneamente siempre que vayan en el mismo sentido y no haya mantenimiento activo ni esperando. Con 10 trenes repartidos en 4 tramos, casi todos los trenes que viajen en el mismo sentido podrán entrar sin esperar por capacidad.
   * **Espera de trenes (tramo):** Se reduce **drásticamente** respecto a capacidad 1. Las esperas quedarán limitadas a los cambios de sentido (esperar a que se vacíe el tramo si viene tráfico en contra) y a las ventanas de mantenimiento (5 ms cada 30 ms).
   * **Espera de trenes (andén):** **Aumenta** ligeramente frente a capacidad 1. Al viajar varios trenes juntos por un tramo, llegarán en ráfagas a la estación destino; como cada estación solo dispone de 2 andenes por sentido, el tercer o cuarto tren del grupo tendrá que esperar antes de entrar al tramo a que se libere un andén en destino (Regla 2).
   * **Tiempo total de ejecución:** **Significativamente menor** que con `--capacidad_tramo 1`, aunque no se reducirá a una cuarta parte debido a los bloqueos por cambio de sentido, el cuello de botella de los 2 andenes por estación y las paradas periódicas de mantenimiento.

### Resultados reales (Hito 2)

| Métrica / Espera | `--capacidad_tramo 1` | `--capacidad_tramo 4` |
| :--- | :--- | :--- |
| Tiempo total de ejecución | *Pendiente* | *Pendiente* |
| Espera media / máx. tren (tramo) | *Pendiente* | *Pendiente* |
| Espera media / máx. tren (andén) | *Pendiente* | *Pendiente* |
| Espera media / máx. mantenimiento | *Pendiente* | *Pendiente* |

**Análisis de desviaciones respecto a la predicción:**
*(A completar tras ejecutar las pruebas en el Hito 2)*

---

## 2. Evidencia de modos de fallo (`--estres 10`) — Hito 2

* **Sin fallo (`./ferrocarril --estres 10`):** *(Pegar salida)*
* **Modo carrera (`./ferrocarril --fallo carrera --estres 10`):** *(Pegar salida)*
* **Modo interbloqueo (`./ferrocarril --fallo interbloqueo --estres 10`):** *(Pegar salida)*
* **Modo inanición (`./ferrocarril --fallo inanicion --estaciones 2 --andenes 30 --trenes 40 --capacidad_tramo 10 --t_tramo 10 --viajes 20 --estres 10`):** *(Pegar salida)*

**¿Por qué la misma configuración falla unas veces sí y otras no?**
*(Explicar el no determinismo del planificador de hilos del sistema operativo y los distintos interleavings entre ejecuciones)*

---

## 3. Competición (`competicion/Producto.cpp`) — Hito 2

* **Hardware utilizado:** MacBook Air (indicar modelo de chip y número de núcleos)
* **Mediana de tiempos (5 ejecuciones en Release):**
  * 1 hilo: *Pendiente*
  * 2 hilos: *Pendiente*
  * 4 hilos: *Pendiente*
  * 8 hilos: *Pendiente*
* **Explicación de la escalabilidad:** *(Completar tras medir)*

---

## 4. Guía técnica de infraestructura base (`base/`) — Módulo 5

Para la integración con `caso/main.cpp`, estas son las claves y utilidades configuradas en la infraestructura base:

* **Claves registradas en `Lanzador` (`caso4`):**
  * `estaciones` (por defecto `5` → 4 tramos de vía única)
  * `andenes` (por defecto `2` por sentido en cada estación)
  * `trenes` (por defecto `10`)
  * `capacidad_tramo` (por defecto `2`)
  * `viajes` (por defecto `30`)
  * `t_tramo` (por defecto `3` ms)
* **Modos de fallo registrados:** `carrera`, `interbloqueo`, `inanicion`
* **Invariantes y vigilancia:**
  * `vigilante.progreso()` debe invocarse cada vez que un tren completa 1 viaje (timeout de 2000 ms).
  * `estadisticas.registrarEspera(...)` para medir esperas de tren (tramo), tren (andén) y mantenimiento.
  * `estadisticas.violacion(...)` si se detectan dos trenes en sentidos opuestos en el mismo tramo o exceso de capacidad.
