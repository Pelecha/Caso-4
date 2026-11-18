

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
