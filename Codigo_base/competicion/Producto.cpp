#include "Producto.h"

#include "Valor.h"

#include <thread>
#include <vector>

// Calculo paralelo del producto modular repartiendo el trabajo en bloques
std::uint64_t producto(int hilos) {
    if (hilos <= 1) {
        std::uint64_t r = 1;
        for (std::uint64_t i = 0; i < N_VALORES; ++i) {
            r = (r * valor(i)) % MODULO;
        }
        return r;
    }

    std::vector<std::thread> trabajadores;
    std::vector<std::uint64_t> resultados(hilos, 1);
    std::uint64_t tamBloque = N_VALORES / hilos;

    // Repartimos el rango [0, N_VALORES) entre los hilos
    for (int t = 0; t < hilos; ++t) {
        std::uint64_t inicio = t * tamBloque;
        std::uint64_t fin = (t == hilos - 1) ? N_VALORES : (t + 1) * tamBloque;

        trabajadores.emplace_back([t, inicio, fin, &resultados]() {
            std::uint64_t prodLocal = 1;
            for (std::uint64_t i = inicio; i < fin; ++i) {
                prodLocal = (prodLocal * valor(i)) % MODULO;
            }
            resultados[t] = prodLocal;
        });
    }

    for (auto& th : trabajadores) {
        th.join();
    }

    // Combinamos los resultados parciales de cada hilo
    std::uint64_t total = 1;
    for (int t = 0; t < hilos; ++t) {
        total = (total * resultados[t]) % MODULO;
    }
    return total;
}
