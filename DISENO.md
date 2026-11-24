# Diseño de Sincronización - Caso 4 (Línea de ferrocarril)

En este documento se describe el diseño de concurrencia y las tablas de predicados de todos los monitores implementados en el proyecto (`Semaforo`, `Tramo` y la cola de pasajeros de `Estacion`).

---

## 1. Monitor `Semaforo`

Implementado en `semaforo/Semaforo.h` y `Semaforo.cpp`. Es un semáforo contador de permisos implementado como un monitor con `std::mutex` y `std::condition_variable`.

| Operación | Espera mientras... (Predicado) | ¿Quién avisa? |
| :--- | :--- | :--- |
| `acquire()` | `_valor == 0` (no hay permisos disponibles) | `release()`, al incrementar el contador con `_cv.notify_one()` |
| `tryAcquireFor(plazo)` | `_valor == 0` (espera como mucho el plazo indicado) | `release()`, al incrementar el contador con `_cv.notify_one()` |
| `release()` | No espera. Incrementa `_valor++` | Avisa con `_cv.notify_one()` a un hilo bloqueado |

---

## 2. Monitor `Tramo`

Implementado en `caso/Tramo.h` y `Tramo.cpp`. Controla el paso de los trenes por un tramo de vía única bidireccional y las paradas de mantenimiento.

Condición de entrada (`puedeEntrar(sentido)`):
El tramo solo es accesible si no hay mantenimiento (`!_mantenimiento`), no se ha superado la capacidad máxima (`_dentro < _capacidad`) y o bien está vacío o los trenes que circulan van en el mismo sentido (`_dentro == 0 || _sentido == sentido`).

| Operación | Espera mientras... (Predicado) | ¿Quién avisa? |
| :--- | :--- | :--- |
| `entrar(sentido)` | `!puedeEntrar(sentido)`<br>Es decir, mientras:<br>• `_mantenimiento == true`, o<br>• `_dentro >= _capacidad`, o<br>• `_dentro > 0 && _sentido != sentido` | • `salir()`: cuando un tren sale del tramo y decrementa `_dentro` (`_cv.notify_all()`)<br>• `abrir()`: cuando el mantenimiento termina y reabre la vía (`_cv.notify_all()`) |
| `cerrarParaMantenimiento()` | `_dentro > 0`<br>(espera a que el tramo quede completamente vacío tras marcar `_mantenimiento = true`) | `salir()`: los trenes que ya estaban dentro van saliendo hasta que `_dentro == 0` (`_cv.notify_all()`) |
| `salir()` | No espera. Decrementa `_dentro` (si llega a 0 resetea `_sentido = 0`) | Avisa con `_cv.notify_all()` a trenes en espera y al mantenimiento |
| `abrir()` | No espera. Marca `_mantenimiento = false` | Avisa con `_cv.notify_all()` a los trenes que esperaban a que terminase el mantenimiento |

---

## 3. Monitor de la Cola de Pasajeros (`Estacion`)

Implementado en `caso/Estacion.h` y `Estacion.cpp`. Gestiona la cola de pasajeros que esperan en cada estación con capacidad máxima de 30 personas.

| Operación | Espera mientras... (Predicado) | ¿Quién avisa? |
| :--- | :--- | :--- |
| `anadirPasajero()` | `_cola >= CAPACIDAD_COLA && !_cerrada`<br>(la cola está llena con 30 personas y la estación sigue abierta) | • `recoger(hueco)`: cuando llega un tren y sube pasajeros, liberando espacio en la cola (`_cv.notify_all()`)<br>• `cerrar()`: cuando finaliza la simulación para que el hilo generador despierte y termine (`_cv.notify_all()`) |
| `recoger(hueco)` | No espera. El tren sube los pasajeros que haya disponibles en ese instante (como máximo `hueco`) | Avisa con `_cv.notify_all()` al hilo generador por si estaba bloqueado por cola llena |
| `cerrar()` | No espera. Marca `_cerrada = true` | Avisa con `_cv.notify_all()` a los hilos generadores |
| `esperando()` | No espera. Lee el tamaño actual de `_cola` bajo exclusión mutua | No avisa |

> **Nota sobre andenes:** El acceso a los andenes de la estación no usa una variable de condición directa, sino semáforos contadores: nuestro `Semaforo` para el sentido positivo y `std::counting_semaphore` para el sentido negativo, inicializados con la capacidad de andenes por sentido.
