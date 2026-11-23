# Guion de defensa - Caso 4 (Línea de ferrocarril)

## 1. Estructura general del proyecto
El proyecto está dentro de `Código base/` y se divide en cuatro bloques principales:
- `base/`: infraestructura común ya dada (gestión de argumentos por consola con `Lanzador`, medición de esperas e invariantes en `Estadisticas`, y control de bloqueos con `Vigilante`).
- `semaforo/` y `prueba_semaforo/`: implementación propia de un semáforo contador usando un monitor (`std::mutex` + `std::condition_variable`) y su test de estrés con 12 hilos.
- `caso/`: simulación de la línea de ferrocarril (`Estacion`, `Tramo`, `Linea` y `main.cpp`).
- `competicion/`: cálculo paralelo del producto modular (`Producto.cpp`) repartiendo el rango entre varios `std::thread`.

---

## 2. Cómo funciona cada archivo clave

### `semaforo/Semaforo.h` y `Semaforo.cpp`
- Guarda un contador entero protegido por un `std::mutex` y una `std::condition_variable`.
- `acquire()`: bloquea el mutex con `std::unique_lock` y espera en `cv.wait` mientras `contador == 0`. Cuando hay permiso disponible, decrementa el contador.
- `release()`: bloquea el mutex, incrementa el contador y llama a `notify_one()` para despertar a un hilo en espera.
- `tryAcquireFor(ms)`: usa `wait_for` con el tiempo límite; si expira sin que el contador sea mayor que 0, devuelve `false` en lugar de quedarse bloqueado.

### `caso/Estacion.h` y `Estacion.cpp`
- **Cola de pasajeros:** funciona como un monitor con capacidad máxima de 30. El hilo generador espera con `condition_variable` si la cola está llena; cuando llega un tren, recoge todos los pasajeros que quepan en su capacidad sin quedarse esperando.
- **Andenes por sentido:** cada estación tiene `A` andenes para sentido `+` (gestionados con nuestro `Semaforo` propio) y `A` andenes para sentido `-` (gestionados con `std::counting_semaphore`). Antes de entrar a un tramo, el tren reserva el andén de destino para evitar quedarse bloqueado dentro de la vía única.

### `caso/Tramo.h` y `Tramo.cpp`
- Implementado como un monitor que controla el acceso a cada tramo de vía única entre dos estaciones.
- **Entrada de un tren:** solo entra si no hay mantenimiento trabajando ni esperando (`mantActivo == false && mantEsperando == 0`), hay menos de `C` (`capacidad_tramo`) trenes dentro y, si ya hay trenes dentro, circulan en su mismo sentido.
- **Entrada de mantenimiento:** marca `mantEsperando` para cortar el paso a nuevos trenes (evitando inanición) y espera en la `condition_variable` hasta que el tramo quede vacío (`trenesDentro == 0`).

### `caso/Linea.h`, `Linea.cpp` y `main.cpp`
- **Ciclo de cada tren:** recoge pasajeros en la estación actual y espera 1 ms $\rightarrow$ reserva andén en la estación siguiente $\rightarrow$ entra al tramo $\rightarrow$ libera el andén origen $\rightarrow$ recorre el tramo (`t_tramo` ms) y sale $\rightarrow$ baja pasajeros y llama a `vigilante.progreso()`.
- **Ciclo de mantenimiento:** cada 30 ms recorre los tramos en sentido inverso ($E-2 \dots 0$), entrando en cada tramo y trabajando 5 ms.
- Al terminar todos los viajes de todos los trenes, se activa el cierre ordenado para detener los hilos de generación de pasajeros y mantenimiento sin dejar hilos colgados.

### `competicion/Producto.cpp`
- Divide el intervalo `[0, 1999999]` en bloques iguales según el número de hilos (`hilos`).
- Cada `std::thread` calcula el producto parcial módulo `1000000007` en una variable local para evitar contención de caché/mutex en cada iteración, y al final se combinan los resultados parciales en el hilo principal.

---

## 3. Explicación de los 3 modos de fallo (`--fallo`)
- **`carrera`:** se rompe la atomicidad al comprobar si el tramo está libre en un bloqueo y actualizar el contador/sentido en otro separado, permitiendo que dos trenes en sentido opuesto entren a la vez y salte una violación de invariante.
- **`interbloqueo`:** el tren entra al tramo sin haber reservado antes el andén en la estación de destino y lo pide cuando ya está dentro del tramo. Si los tramos y andenes se llenan en sentidos opuestos, ningún tren puede avanzar ni liberar sitio y salta el `Vigilante` por falta de progreso.
- **`inanicion`:** los trenes siguen entrando al tramo aunque el mantenimiento esté esperando. Con tráfico continuo en el mismo sentido, el contador de trenes dentro nunca llega a 0 y el mantenimiento no logra entrar.

---

## 4. Comandos rápidos para enseñar en la defensa
- Compilar sin warnings: `cmake -B build -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic" && cmake --build build --config Release`
- Test del semáforo: `./build/prueba_semaforo` (debe imprimir `OK`)
- Ejecución normal bajo estrés: `./build/ferrocarril --estres 10`
- Provocar fallo de carrera: `./build/ferrocarril --fallo carrera --estres 10`
- Validar competición: `./build/competicion --comprobar`
