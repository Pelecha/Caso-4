#include "Linea.h"

#include "Config.h"
#include "Estadisticas.h"
#include "Traza.h"
#include "Vigilante.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

static const int PASAJEROS_POR_TREN = 20;

Linea::Linea(const Contexto& ctx, int numEstaciones, int andenes, int capacidadTramo, int tTramoMs)
    : _ctx(ctx),
      _numEstaciones(numEstaciones),
      _tTramoMs(tTramoMs),
      _interbloqueo(ctx.config.fallo("interbloqueo")) {
    for (int e = 0; e < numEstaciones; ++e) {
        _estaciones.push_back(std::make_unique<Estacion>(ctx, e, andenes));
    }
    for (int t = 0; t < numEstaciones - 1; ++t) {
        _tramos.push_back(std::make_unique<Tramo>(ctx, t, capacidadTramo));
    }
}

void Linea::hiloTren(int k, int viajes) {
    int E = _numEstaciones;

    // Posicion inicial segun la formula del enunciado
    int p = k % (2 * (E - 1));
    int estacion;
    int sentido;
    if (p < E - 1) {
        estacion = p;
        sentido = +1;
    } else {
        estacion = 2 * (E - 1) - p;
        sentido = -1;
    }
    _estaciones[estacion]->reservarAnden(sentido);

    for (int v = 0; v < viajes; ++v) {
        // a. Recoge pasajeros y espera 1 ms
        int pasajeros = _estaciones[estacion]->recoger(PASAJEROS_POR_TREN);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // b. Anden en la estacion siguiente y entrada en el tramo
        int siguiente = estacion + sentido;
        int sentidoNuevo = sentido;
        if (siguiente == E - 1) {
            sentidoNuevo = -1;
        } else if (siguiente == 0) {
            sentidoNuevo = +1;
        }
        int tramo = std::min(estacion, siguiente);

        if (_interbloqueo) {
            // ERROR deliberado (--fallo interbloqueo): entra al tramo ANTES de reservar el anden de destino
            _tramos[tramo]->entrar(sentido);
            _estaciones[estacion]->soltarAnden(sentido);
            _estaciones[siguiente]->reservarAnden(sentidoNuevo);
        } else {
            // Correcto: reserva primero el anden de la estacion siguiente para no atascar el tramo
            _estaciones[siguiente]->reservarAnden(sentidoNuevo);
            _tramos[tramo]->entrar(sentido);
            _estaciones[estacion]->soltarAnden(sentido);
        }

        // c. Recorre el tramo y sale
        _ctx.traza.evento("tren " + std::to_string(k), "recorre el tramo " + std::to_string(tramo));
        std::this_thread::sleep_for(std::chrono::milliseconds(_tTramoMs));
        _tramos[tramo]->salir();

        // d. Bajan los pasajeros
        _ctx.estadisticas.sumar("transportados", pasajeros);
        estacion = siguiente;
        sentido = sentidoNuevo;

        _ctx.estadisticas.sumar("viajes", 1);
        _ctx.vigilante.progreso();
    }

    _estaciones[estacion]->soltarAnden(sentido);
    _ctx.traza.evento("tren " + std::to_string(k), "termina");
}

void Linea::hiloMantenimiento() {
    int tramo = _numEstaciones - 2;
    while (true) {
        {
            std::unique_lock<std::mutex> lk(_m);
            if (_cv.wait_for(lk, std::chrono::milliseconds(30), [&] { return _fin; })) {
                break;
            }
        }
        _tramos[tramo]->cerrarParaMantenimiento();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));  // trabajo de 5 ms
        _tramos[tramo]->abrir();
        _ctx.estadisticas.sumar("trabajos de mantenimiento", 1);

        tramo = (tramo == 0) ? _numEstaciones - 2 : tramo - 1;
    }
    _ctx.traza.evento("mantenimiento", "termina");
}

void Linea::avisarMantenimiento() {
    std::lock_guard<std::mutex> lk(_m);
    _fin = true;
    _cv.notify_all();
}

void Linea::cerrarEstaciones() {
    for (auto& e : _estaciones) {
        e->cerrar();
    }
}

void Linea::comprobarFinal() {
    long esperando = 0;
    for (auto& e : _estaciones) {
        esperando += e->esperando();
    }
    _ctx.estadisticas.sumar("esperando al final", esperando);

    long generados = _ctx.estadisticas.valor("pasajeros generados");
    long transportados = _ctx.estadisticas.valor("transportados");
    if (generados != transportados + esperando) {
        _ctx.estadisticas.violacion("pasajeros generados = " + std::to_string(generados) +
                                    ", transportados + esperando = " + std::to_string(transportados + esperando));
    }

    if (_ctx.config.fallo("inanicion")) {
        long trabajos = _ctx.estadisticas.valor("trabajos de mantenimiento");
        if (trabajos == 0) {
            _ctx.estadisticas.violacion("inanicion: el mantenimiento no ha podido realizar ningun trabajo");
        }
    }
}
