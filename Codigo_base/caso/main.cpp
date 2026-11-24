#include "Config.h"
#include "Contexto.h"
#include "Estadisticas.h"
#include "Lanzador.h"
#include "Linea.h"
#include "Traza.h"
#include "Vigilante.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

bool ejecutarSimulacion(const Config& config, Estadisticas& estadisticas, Traza& traza, const std::string& etiqueta) {
    int estaciones = std::max(2, config.entero("estaciones", 5));
    int andenes = config.entero("andenes", 2);
    int trenes = config.entero("trenes", 10);
    int capacidadTramo = config.entero("capacidad_tramo", 2);
    int viajes = config.entero("viajes", 30);
    int tTramo = config.entero("t_tramo", 3);

    Vigilante vigilante(std::chrono::milliseconds(config.entero("vigilante", 2000)), estadisticas, etiqueta);
    Contexto ctx{config, estadisticas, traza, vigilante};
    Linea linea(ctx, estaciones, andenes, capacidadTramo, tTramo);

    vigilante.iniciar();

    // Lanzar los hilos: generadores (uno por estacion), mantenimiento y trenes.
    std::vector<std::thread> generadores;
    for (int e = 0; e < estaciones; ++e) {
        generadores.emplace_back(&Estacion::hiloGenerador, &linea.estacion(e));
    }
    std::thread mantenimiento(&Linea::hiloMantenimiento, &linea);
    std::vector<std::thread> hilosTrenes;
    for (int k = 0; k < trenes; ++k) {
        hilosTrenes.emplace_back(&Linea::hiloTren, &linea, k, viajes);
    }

    // Cierre ordenado: esperamos a que terminen los trenes, y luego avisamos a mantenimiento y generadores.
    for (auto& t : hilosTrenes) {
        t.join();
    }
    linea.avisarMantenimiento();
    linea.cerrarEstaciones();
    mantenimiento.join();
    for (auto& g : generadores) {
        g.join();
    }
    vigilante.detener();

    // Comprobacion de invariantes finales
    linea.comprobarFinal();
    return estadisticas.sinViolaciones();
}

int main(int argc, char* argv[]) {
    Config config(argc, argv);
    Lanzador lanzador("ferrocarril",
                      {"estaciones", "andenes", "trenes", "capacidad_tramo", "viajes", "t_tramo"},
                      {"carrera", "interbloqueo", "inanicion"});
    return lanzador.ejecutar(config, ejecutarSimulacion);
}
