#include "Estacion.h"

#include "Estadisticas.h"
#include "Traza.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

static const int CAPACIDAD_COLA = 30;

Estacion::Estacion(const Contexto& ctx, int id, int andenes)
    : _ctx(ctx), _id(id), _andenesMas(andenes), _andenesMenos(andenes) {}

void Estacion::reservarAnden(int sentido) {
    auto inicio = std::chrono::steady_clock::now();
    if (sentido > 0) {
        _andenesMas.acquire();
    } else {
        _andenesMenos.acquire();
    }
    _ctx.estadisticas.registrarEspera("tren (andén)", std::chrono::steady_clock::now() - inicio);
}

void Estacion::soltarAnden(int sentido) {
    if (sentido > 0) {
        _andenesMas.release();
    } else {
        _andenesMenos.release();
    }
}

int Estacion::recoger(int hueco) {
    std::lock_guard<std::mutex> lk(_m);
    int n = std::min(hueco, _cola);
    _cola -= n;
    _cv.notify_all();  // avisamos al generador porque ahora hay sitio libre
    return n;
}

bool Estacion::anadirPasajero() {
    std::unique_lock<std::mutex> lk(_m);
    _cv.wait(lk, [&] { return _cola < CAPACIDAD_COLA || _cerrada; });
    if (_cerrada) {
        return false;
    }
    ++_cola;
    _ctx.estadisticas.sumar("pasajeros generados", 1);
    return true;
}

void Estacion::cerrar() {
    std::lock_guard<std::mutex> lk(_m);
    _cerrada = true;
    _cv.notify_all();
}

int Estacion::esperando() {
    std::lock_guard<std::mutex> lk(_m);
    return _cola;
}

void Estacion::hiloGenerador() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (!anadirPasajero()) {
            break;
        }
    }
    _ctx.traza.evento("estacion " + std::to_string(_id), "generador termina");
}
