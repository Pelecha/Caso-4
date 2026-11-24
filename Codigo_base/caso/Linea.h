#pragma once

#include "Contexto.h"
#include "Estacion.h"
#include "Tramo.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

class Linea {
public:
    Linea(const Contexto& ctx, int numEstaciones, int andenes, int capacidadTramo, int tTramoMs);

    void hiloTren(int k, int viajes);
    void hiloMantenimiento();

    void avisarMantenimiento();
    void cerrarEstaciones();
    void comprobarFinal();

    Estacion& estacion(int i) { return *_estaciones[i]; }

private:
    const Contexto& _ctx;
    int _numEstaciones;
    int _tTramoMs;
    bool _interbloqueo;
    std::vector<std::unique_ptr<Estacion>> _estaciones;
    std::vector<std::unique_ptr<Tramo>> _tramos;

    std::mutex _m;
    std::condition_variable _cv;
    bool _fin = false;
};
