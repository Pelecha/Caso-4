#include "Tramo.h"

#include "Config.h"
#include "Estadisticas.h"

#include <chrono>
#include <string>
#include <thread>

Tramo::Tramo(const Contexto& ctx, int id, int capacidad)
    : _ctx(ctx),
      _id(id),
      _capacidad(capacidad),
      _carrera(ctx.config.fallo("carrera")),
      _inanicion(ctx.config.fallo("inanicion")) {}

bool Tramo::puedeEntrar(int sentido) const {
    bool mantenimientoBloquea = _mantenimiento && !_inanicion;
    return !mantenimientoBloquea && _dentro < _capacidad && (_dentro == 0 || _sentido == sentido);
}

void Tramo::entrar(int sentido) {
    auto inicio = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lk(_m);

    _cv.wait(lk, [&] { return puedeEntrar(sentido); });

    if (_carrera) {
        lk.unlock();
        std::this_thread::yield();
        lk.lock();
    }

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
    _mantenimiento = true;
    _cv.wait(lk, [&] { return _dentro == 0; });

    auto duracion = std::chrono::steady_clock::now() - inicio;
    _ctx.estadisticas.registrarEspera("mantenimiento (tramo vacío)", duracion);

    if (_inanicion) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duracion).count();
        if (ms > 50) {
            _ctx.estadisticas.violacion("tramo " + std::to_string(_id) +
                                        ": mantenimiento sufre inanicion esperando tramo vacio (" +
                                        std::to_string(ms) + " ms)");
        }
    }
}

void Tramo::abrir() {
    std::lock_guard<std::mutex> lk(_m);
    _mantenimiento = false;
    _cv.notify_all();
}
