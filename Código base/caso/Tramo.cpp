#include "Tramo.h"

#include "Config.h"
#include "Estadisticas.h"

#include <chrono>
#include <string>
#include <thread>

Tramo::Tramo(const Contexto& ctx, int id, int capacidad)
    : _ctx(ctx), _id(id), _capacidad(capacidad), _carrera(ctx.config.fallo("carrera")) {}

bool Tramo::puedeEntrar(int sentido) const {
    return !_mantenimiento && _dentro < _capacidad && (_dentro == 0 || _sentido == sentido);
}

void Tramo::entrar(int sentido) {
    auto inicio = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lk(_m);

    // Sección crítica 1: comprobar si puede entrar.
    _cv.wait(lk, [&] { return puedeEntrar(sentido); });

    if (_carrera) {
        // ERROR deliberado (--fallo carrera): se suelta el cerrojo entre comprobar y apuntar.
        // Otro tren puede haber entrado en ese hueco y el tramo ya no cumple lo que comprobamos.
        lk.unlock();
        std::this_thread::yield();
        lk.lock();
    }

    // Sección crítica 2: apuntar la entrada.
    _ctx.estadisticas.registrarEspera("tren (tramo)", std::chrono::steady_clock::now() - inicio);
    if (_dentro > 0 && _sentido != sentido) {
        _ctx.estadisticas.violacion("tramo " + std::to_string(_id) + ": trenes en sentidos opuestos");
    }
    _sentido = sentido;
    ++_dentro;
    if (_dentro > _capacidad) {
        _ctx.estadisticas.violacion("tramo " + std::to_string(_id) + ": " + std::to_string(_dentro) +
                                    " trenes, capacidad " + std::to_string(_capacidad));
    }
}

void Tramo::salir() {
    std::lock_guard<std::mutex> lk(_m);
    --_dentro;
    if (_dentro == 0) {
        _sentido = 0;
    }
    _cv.notify_all();
}

void Tramo::cerrarParaMantenimiento() {
    auto inicio = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lk(_m);
    _mantenimiento = true;  // desde ahora no entra ningún tren
    _cv.wait(lk, [&] { return _dentro == 0; });
    _ctx.estadisticas.registrarEspera("mantenimiento (tramo vacío)", std::chrono::steady_clock::now() - inicio);
}

void Tramo::abrir() {
    std::lock_guard<std::mutex> lk(_m);
    _mantenimiento = false;
    _cv.notify_all();
}
