#pragma once

#include "Contexto.h"

#include <condition_variable>
#include <mutex>

// Un tramo de via unica: varios trenes en el mismo sentido, nunca en sentidos opuestos.
class Tramo {
public:
    Tramo(const Contexto& ctx, int id, int capacidad);

    void entrar(int sentido);
    void salir();
    void cerrarParaMantenimiento();
    void abrir();

private:
    bool puedeEntrar(int sentido) const;

    const Contexto& _ctx;
    int _id;
    int _capacidad;
    bool _carrera;
    bool _inanicion;
    std::mutex _m;
    std::condition_variable _cv;
    int _dentro = 0;        // numero de trenes en el tramo
    int _sentido = 0;       // sentido actual (+1 o -1, 0 si esta vacio)
    bool _mantenimiento = false; // indica si el mantenimiento esta esperando o trabajando
};
