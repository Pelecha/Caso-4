# Guion de Defensa - Caso 4 (Línea de Ferrocarril)

## 👥 Integrantes del Equipo
* Alejandro Pelechá
* Viviana Chillagana
* Hector Palomino
* Brian Diaz
* Nicolas Salazar

---

## 1. Estructura General del Proyecto

El código fuente se encuentra organizado dentro de la carpeta `Codigo_base/` en los siguientes módulos:
- `base/`: Infraestructura proporcionada para la práctica:
  - `Lanzador`: parsea argumentos de consola, opciones y modos de fallo (`--fallo`, `--estres`).
  - `Estadisticas`: mide tiempos de espera y registra violaciones de invariantes de forma segura entre hilos.
  - `Vigilante`: hilo guardián que detecta interbloqueos si transcurren más de 2000 ms sin progreso (`vigilante.progreso()`).
- `semaforo/` y `prueba_semaforo/`: Monitor propio `Semaforo` (contador de permisos con `std::mutex` y `std::condition_variable`) y su suite de pruebas con 12 hilos concurrentes.
- `caso/`: Simulación del ferrocarril:
  - `Estacion`: andenes (semáforos) y monitor de la cola de pasajeros.
  - `Tramo`: monitor que regula el acceso exclusivo/concurrente a la vía única bidireccional.
  - `Linea`: orquesta los hilos de trenes, generadores de pasajeros y mantenimiento, junto con el cierre ordenado e invariantes.
  - `main.cpp`: punto de entrada que define los parámetros por defecto y lanza la simulación.
- `competicion/`: Cálculo paralelo del producto modular en `Producto.cpp`.

---

## 2. Monitores y Sincronización

### `Semaforo` (Monitor de Permisos)
- Protege un contador entero `_valor`.
- `acquire()`: espera en `_cv.wait` mientras `_valor == 0`. Al despertar decrementa `_valor`.
- `release()`: incrementa `_valor` y despierta a un hilo con `_cv.notify_one()`.
- `tryAcquireFor(plazo)`: espera con temporizador `_cv.wait_for`. Devuelve `false` si vence el plazo sin permisos disponibles.

### `Tramo` (Monitor de Vía Única)
- Controla el tránsito bidireccional entre estaciones adyacentes.
- **Predicado de entrada (`puedeEntrar`):**
  Un tren solo entra si:
  `!_mantenimiento && _dentro < _capacidad && (_dentro == 0 || _sentido == sentido)`
- **Mantenimiento:**
  Al invocar `cerrarParaMantenimiento()`, marca `_mantenimiento = true` cortando el acceso a nuevos trenes y espera en `_cv.wait` a que `_dentro == 0`.
- **Salida:**
  `salir()` decrementa `_dentro` (reseteando el sentido a 0 si queda vacío) y hace `_cv.notify_all()`.
  `abrir()` reabre el tramo tras la revisión y notifica con `_cv.notify_all()`.

### `Estacion` (Andenes y Monitor de Pasajeros)
- **Andenes:** Gestionados mediante semáforos:
  - Sentido positivo (`+1`): nuestro `Semaforo` propio.
  - Sentido negativo (`-1`): `std::counting_semaphore`.
- **Cola de pasajeros:**
  Monitor con capacidad máxima de 30 personas.
  - `anadirPasajero()`: el generador espera si `_cola >= 30` mientras la estación no esté cerrada.
  - `recoger(hueco)`: el tren recoge los pasajeros disponibles sin esperar y despierta al generador con `_cv.notify_all()`.
  - `cerrar()`: despierta a los generadores para que terminen limpiamente sus hilos.

---

## 3. Ciclo de Vida de los Hilos y Cierre Ordenado

1. **Hilos Trenes:**
   - Cada tren empieza en su estación inicial asignada por la fórmula del enunciado y reserva andén.
   - En cada viaje: recoge pasajeros (1 ms) $\rightarrow$ reserva andén en la estación destino $\rightarrow$ entra al tramo $\rightarrow$ libera el andén origen $\rightarrow$ recorre el tramo ($t\_tramo$ ms) $\rightarrow$ sale del tramo $\rightarrow$ bajan los pasajeros y llama a `vigilante.progreso()`.
2. **Hilo de Mantenimiento:**
   - Cada 30 ms recorre los tramos en sentido inverso ($E-2 \dots 0$), cerrando cada tramo, trabajando 5 ms y volviéndolo a abrir.
3. **Cierre Ordenado:**
   - Primero se espera a que todos los trenes completen sus viajes (`t.join()`).
   - Se avisa al mantenimiento (`linea.avisarMantenimiento()`) y se cierran las estaciones (`linea.cerrarEstaciones()`).
   - Se espera al hilo de mantenimiento y a los hilos generadores (`join()`).
   - Se detiene el `Vigilante`.
4. **Comprobación de Invariantes:**
   - `pasajeros generados == transportados + esperando en andenes`.
   - Ningún tramo puede contener trenes en sentidos contrarios ni superar su capacidad.

---

## 4. Modos de Fallo (`--fallo`) y el Planificador

- **`carrera` (`--fallo carrera`):**
  - **Mecanismo:** En `Tramo::entrar`, tras comprobar con éxito que la vía está libre, se suelta temporalmente el mutex (`lk.unlock()`), se cede la CPU (`yield()`) y se vuelve a coger el cerrojo antes de actualizar contadores.
  - **Efecto:** Otro tren entra en el hueco temporal, provocando que coincidan trenes en sentidos opuestos o se supere la capacidad (invariante roto).
  - **No determinismo:** En cada ejecución fallan tramos distintos (a veces tramo 1, otras tramo 2 o 3) según qué hilo elija el planificador del SO durante la cesión de CPU.

- **`interbloqueo` (`--fallo interbloqueo`):**
  - **Mecanismo:** El tren entra en el tramo de vía única *antes* de haber reservado el andén en la estación de destino.
  - **Efecto:** Si entran trenes en sentidos opuestos o convergen en una estación con andenes llenos, se genera una espera circular: los trenes del tramo esperan andén, y los trenes del andén esperan tramo.
  - **Detección:** Los trenes dejan de progresar y el `Vigilante` aborta tras 2000 ms.

- **`inanicion` (`--fallo inanicion`):**
  - **Mecanismo:** Los trenes ignoran la bandera `_mantenimiento` y continúan entrando al tramo sin ceder el paso.
  - **Efecto:** En escenarios de alto tráfico, el tramo casi nunca llega a estar vacío (`_dentro == 0`), de modo que el mantenimiento sufre demoras extremas esperando para cerrar la vía.

---

## 5. Competición Paralela (`Producto.cpp`)

- **Objetivo:** Calcular $( \prod_{i=0}^{N-1} valor(i) ) \pmod{1000000007}$ con $N = 2.000.000$.
- **Diseño:**
  - Se reparte el rango $[0, 2000000)$ en $H$ bloques continuos equitativos.
  - Cada hilo calcula su producto parcial de forma local, evitando colisiones en memoria caché (sin *false sharing*) y sin necesidad de bloqueos por mutex durante el cómputo.
  - Al terminar los hilos (`join()`), el hilo principal multiplica los $H$ resultados parciales.
- **Rendimiento:**
  - 1 hilo: ~5607 ms
  - 2 hilos: ~4241 ms
  - 4 hilos: ~2479 ms (speedup ~2.26x)
  - 8 hilos: ~1332 ms (speedup ~4.21x)

---

## 6. Comandos Rápidos para la Defensa

```bash
# Compilar todo en Release
cmake -B build -S Codigo_base -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 1. Comprobar test del Semaforo (debe imprimir OK)
./build/prueba_semaforo/Release/prueba_semaforo.exe

# 2. Simulación normal con estrés (10/10 OK)
./build/caso/Release/ferrocarril.exe --estres 10

# 3. Demostrar fallo de carrera
./build/caso/Release/ferrocarril.exe --fallo carrera --estres 10

# 4. Demostrar fallo de interbloqueo (salta el Vigilante)
./build/caso/Release/ferrocarril.exe --fallo interbloqueo --estres 10

# 5. Demostrar fallo de inanición
./build/caso/Release/ferrocarril.exe --fallo inanicion --estaciones 2 --andenes 30 --trenes 40 --capacidad_tramo 10 --t_tramo 10 --viajes 20 --estres 10

# 6. Validar corrección de la competición
./build/competicion/Release/competicion.exe --comprobar
```
