#pragma once

#include "Contexto.h"
#include "Estacion.h"
#include "Tramo.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

// La línea: estaciones, tramos, y los hilos de trenes y de mantenimiento.
class Linea {
public:
    Linea(const Contexto& ctx, int numEstaciones, int andenes, int capacidadTramo, int tTramoMs);

    Estacion& estacion(int i) { return *_estaciones[i]; }

    void hiloTren(int k, int viajes);
    void hiloMantenimiento();

    void avisarMantenimiento();  // para el cierre
    void cerrarEstaciones();     // para el cierre
    void comprobarFinal();       // invariante: generados = transportados + esperando

private:
    const Contexto& _ctx;
    int _numEstaciones;
    int _tTramoMs;
    std::vector<std::unique_ptr<Estacion>> _estaciones;
    std::vector<std::unique_ptr<Tramo>> _tramos;

    std::mutex _m;
    std::condition_variable _cv;
    bool _fin = false;
};
