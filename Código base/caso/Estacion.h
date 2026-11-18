#pragma once

#include "Contexto.h"
#include "Semaforo.h"

#include <condition_variable>
#include <mutex>
#include <semaphore>

// Una estación: andenes de cada sentido y cola de pasajeros.
class Estacion {
public:
    Estacion(const Contexto& ctx, int id, int andenes);

    void reservarAnden(int sentido);  // +1: Semaforo propio, -1: std::counting_semaphore
    void soltarAnden(int sentido);

    int recoger(int hueco);   // el tren se lleva los que haya (como mucho hueco), sin esperar
    bool anadirPasajero();    // espera si la cola está llena. false si la estación se ha cerrado
    void cerrar();
    int esperando();

    void hiloGenerador();     // un pasajero cada 1 ms

private:
    const Contexto& _ctx;
    int _id;
    Semaforo _andenesMas;
    std::counting_semaphore<> _andenesMenos;

    std::mutex _m;
    std::condition_variable _cv;
    int _cola = 0;
    bool _cerrada = false;
};
