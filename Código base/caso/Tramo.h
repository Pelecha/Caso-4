#pragma once

#include "Contexto.h"

#include <condition_variable>
#include <mutex>

// Un tramo de vía única: varios trenes en el mismo sentido, nunca en sentidos opuestos.
class Tramo {
public:
    Tramo(const Contexto& ctx, int id, int capacidad);

    void entrar(int sentido);  // sentido: +1 o -1. Con --fallo carrera, comprueba y apunta por separado
    void salir();
    void cerrarParaMantenimiento();  // espera a que el tramo esté vacío
    void abrir();

private:
    bool puedeEntrar(int sentido) const;  // se llama con el mutex cogido

    const Contexto& _ctx;
    int _id;
    int _capacidad;
    bool _carrera;
    std::mutex _m;
    std::condition_variable _cv;
    int _dentro = 0;        // trenes dentro del tramo
    int _sentido = 0;       // sentido de los trenes de dentro (0 si está vacío)
    bool _mantenimiento = false;  // true desde que el mantenimiento empieza a esperar hasta que termina
};
