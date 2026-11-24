#include "Producto.h"

#include "Valor.h"

#include <thread>
#include <vector>

// Version paralela repartiendo el rango entre los hilos
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

    for (int t = 0; t < hilos; ++t) {
        std::uint64_t inicio = t * tamBloque;
        std::uint64_t fin = (t + 1) * tamBloque;

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

    std::uint64_t total = 1;
    for (int t = 0; t < hilos; ++t) {
        total = (total * resultados[t]) % MODULO;
    }
    return total;
}
