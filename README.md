# Caso-4

# Programación Concurrente y Distribuida - Trabajo en Equipo
## Caso 4: Línea de ferrocarril

## 👥 Integrantes del Equipo
* [Alejandro Pelechá]
* [Viviana Chillagana]
* [Hector Palomino]
* [Brian Diaz]
* [Nicolas Salazar]

---

## 📋 Descripción del Proyecto
Este repositorio tiene el **Caso 4** de la asignatura de Programación Concurrente y Distribuida El codigo simula una línea de ferrocarril compuesta por varias estaciones y tramos de vía única bidireccionales por los que circulan trenes.Se gestionan pasajeros, andenes mediante semáforos, y tareas periódicas de mantenimiento.

El proyecto implementa estrictamente:
* **Hilos independientes** para trenes, generadores de pasajeros por estación y el hilo de mantenimiento.
* **Exclusión mutua con RAII** (`std::lock_guard`, `std::unique_lock`).
* **Monitores con variables de condición** (para los tramos de vía y colas de pasajeros).
* **Clase `Semaforo` propia** implementada como monitor (`mutex` + variable de condición).

---

## 🛠️ Compilación

El proyecto utiliza **CMake** y compila sin avisos bajo los flags estrictos requeridos (`-Wall -Wextra -Wpedantic`).

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## ▶️ Ejecución

Una vez compilado el proyecto, se puede ejecutar la simulación de la línea de ferrocarril:

```bash
./ferrocarril
```

También se pueden modificar los parámetros de la simulación:

```bash
./ferrocarril --estaciones 5 --andenes 2 --trenes 10 --capacidad_tramo 2 --viajes 30 --t_tramo 3
```

Para mostrar los eventos de la simulación:

```bash
./ferrocarril --traza
```

---

## 🧪 Pruebas

Para ejecutar las pruebas de la clase `Semaforo`:

```bash
./prueba_semaforo
```

Para realizar diez ejecuciones de la simulación:

```bash
./ferrocarril --estres 10
```

También se pueden probar los diferentes modos de fallo:

**Carrera:**

```bash
./ferrocarril --fallo carrera --estres 10
```

**Interbloqueo:**

```bash
./ferrocarril --fallo interbloqueo
```

**Inanición:**

```bash
./ferrocarril --fallo inanicion --estaciones 2 --andenes 30 --trenes 40 --capacidad_tramo 10 --t_tramo 10 --viajes 20 --estres 10
```

Estos comandos permiten comprobar los problemas de concurrencia que pueden aparecer durante la ejecución.

---

## ⚡ Competición y medición de rendimiento

Para medir el rendimiento del programa se utiliza el ejecutable `competicion`, que permite realizar el cálculo con diferentes números de hilos.

Se realizan cinco mediciones para cada configuración:

**1 hilo:**

```bash
./competicion --hilos 1 --veces 5 --comprobar
```

**2 hilos:**

```bash
./competicion --hilos 2 --veces 5 --comprobar
```

**4 hilos:**

```bash
./competicion --hilos 4 --veces 5 --comprobar
```

**8 hilos:**

```bash
./competicion --hilos 8 --veces 5 --comprobar
```

El parámetro `--hilos` indica el número de hilos utilizados y `--veces` el número de mediciones. El parámetro `--comprobar` verifica que el resultado obtenido coincide con el cálculo secuencial.

### Resultados obtenidos

| Número de hilos | Mediana |
|---|---:|
| 1 | 5607.93 ms |
| 2 | 4241.29 ms |
| 4 | 2479.73 ms |
| 8 | 1332.07 ms |

Como se puede observar, al aumentar el número de hilos disminuye el tiempo de ejecución, ya que el trabajo se reparte entre varios hilos.

Para realizar estas mediciones se ha utilizado la configuración `Release`.

---

## 📄 Documentación adicional

En el archivo `EXPERIMENTOS.md` se pueden consultar los experimentos realizados, las pruebas de fallos y los resultados obtenidos en la competición.

