#include <iostream>     // Para mostrar mensajes por la consola (std::cout)
#include <thread>       // Para poder crear y gestionar hilos (std::thread)
#include <vector>       // Para guardar los hilos en un vector dinámico y controlarlos fácilmente
#include <chrono>       // Para manejar tiempos y pausas (std::this_thread::sleep_for)
#include "Semaforo.h"   // Importa la clase Semaforo programada por tu compañero

//El semaforo con capacidad 3 se crea dentro de main y se pasa a los hilos

void tareaConcurrente(Semaforo& semaforo, int idHilo) {
    const int iteraciones = 20; //Cada hilo repite 20 iteraciones
    
    for (int iter = 0; iter < iteraciones; ++iter) {
        // Probamos la adquisición del semáforo
        semaforoPrueba.acquire(); // Si el semáforo está en 0, el hilo se queda aquí esperando dormido.

        // Simulamos un trabajo dentro de la sección crítica durante 300ms
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Liberamos el semáforo para que otro hilo que esté esperando pueda entrar
        semaforoPrueba.release();
    }

    std::cout << "[Hilo " << idHilo << "] ha completado las 20 iteraciones.\n";

}

int main() {
    std::cout << "=== INICIO DE LA PRUEBA DE SEMÁFORO (Módulo 3) ===\n";

    const int numeroHilos = 5;
    std::vector<std::thread> hilos;

    // Lanzamos varios hilos concurrentes
    for (int i = 0; i < numeroHilos; ++i) {
        hilos.emplace_back(tareaConcurrente, i + 1);
    }

    // Esperamos a que todos los hilos terminen de ejecutarse antes de cerrar el programa
    for (auto& t : hilos) {
        t.join();
    }

    std::cout << "=== PRUEBA DE SEMÁFORO FINALIZADA CON ÉXITO ===\n";
    return 0;
}