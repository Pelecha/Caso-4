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
