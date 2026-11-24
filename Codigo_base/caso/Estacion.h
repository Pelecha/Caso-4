#pragma once

#include "Contexto.h"
#include "Semaforo.h"

#include <condition_variable>
#include <mutex>
#include <semaphore>

// Una estacion con andenes para cada sentido y una cola de pasajeros con monitor
class Estacion {
public:
    Estacion(const Contexto& ctx, int id, int andenes);

    void reservarAnden(int sentido);  // sentido +1 usa Semaforo propio, -1 usa std::counting_semaphore
    void soltarAnden(int sentido);

    int recoger(int hueco);   // el tren recoge los pasajeros que quepan
    bool anadirPasajero();    // espera si la cola esta llena (maximo 30)
    void cerrar();
    int esperando();

    void hiloGenerador();     // genera pasajeros periodicamente

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
