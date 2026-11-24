#include <iostream>     // Para mostrar mensajes por la consola (std::cout)
#include <thread>       // Para poder crear y gestionar hilos (std::thread)
#include <vector>       // Para guardar los hilos en un vector dinámico y controlarlos fácilmente
#include <chrono>       // Para manejar tiempos y pausas (std::this_thread::sleep_for)
#include "Semaforo.h"   // Importa la clase Semaforo programada por tu compañero
#include <mutex>        // Para medir cuantos hilos entran a la vez
#include <semaphore>    // Para probar tambien el semaforo estandar 

//El semaforo con capacidad 3 se crea dentro de main y se pasa a los hilos

void tareaConcurrente(Semaforo& semaforo, int idHilo, std::mutex& mutexContador, int& dentro, int& maximoDentro) {
    const int iteraciones = 20; //Cada hilo repite 20 iteraciones
    
    for (int iter = 0; iter < iteraciones; ++iter) {
        // Probamos la adquisición del semáforo
        semaforo.acquire(); // Si el semáforo está en 0, el hilo se queda aquí esperando dormido.

        // Cuenta cuantos hilos estan dentro al mismo tiempo
        {
            std::lock_guard<std::mutex> lock(mutexContador);
            dentro++;

            if (dentro > maximoDentro) {
                maximoDentro = dentro;
            }
        }

        // Simulamos un trabajo dentro de la sección crítica durante 300ms
        std::this_thread::sleep_for(std::chrono::microseconds(300));

        // Este hilo sale de la zona protegida
        {
            std::lock_guard<std::mutex> lock(mutexContador);
            dentro--;
        }

        // Liberamos el semáforo para que otro hilo que esté esperando pueda entrar
        semaforo.release();
    }

    std::cout << "[Hilo " << idHilo << "] ha completado las 20 iteraciones.\n";

}

void tareaConcurrenteEstandar(std::counting_semaphore<3>& semaforo, int idHilo, std::mutex& mutexContador, int& dentro, int& maximoDentro) {
    const int iteraciones = 20; //Cada hilo repite 20 iteraciones

    for (int iter = 0; iter < iteraciones; ++iter) {
        // Probamos la adquisición del semáforo
        semaforo.acquire(); // Si el semáforo está en 0, el hilo se queda aquí esperando dormido.

        // Cuenta cuantos hilos estan dentro al mismo tiempo
        {
            std::lock_guard<std::mutex> lock(mutexContador);
            dentro++;

            if (dentro > maximoDentro) {
                maximoDentro = dentro;
            }
        }

        // Simulamos un trabajo dentro de la sección crítica durante 300ms
        std::this_thread::sleep_for(std::chrono::microseconds(300));

        // Este hilo sale de la zona protegida
        {
            std::lock_guard<std::mutex> lock(mutexContador);
            dentro--;
        }

        // Liberamos el semáforo para que otro hilo que esté esperando pueda entrar
        semaforo.release();
    }

    std::cout << "[Hilo " << idHilo << "] ha completado las 20 iteraciones.\n";

}

int probarSemaforoPropio() {
    const int numeroHilos = 12; //Se lanzan 12 hilos
    const int capacidadSemaforo = 3; //Capacidqad 3

    Semaforo semaforoCapacidad(capacidadSemaforo);
    std::mutex mutexContador;
    int dentro = 0;
    int maximoDentro = 0;
    std::vector<std::thread> hilos;

    for (int i = 0; i < numeroHilos; ++i) {
        hilos.emplace_back([&, i]() {
            tareaConcurrente(semaforoCapacidad, i + 1, mutexContador, dentro, maximoDentro);
            });
    }

    for (std::thread& hilo : hilos) {
        hilo.join();
    }

    std::cout << "Maximo de hilos dentro: " << maximoDentro << " de " << capacidadSemaforo << "\n";

    if (maximoDentro > capacidadaSemaforo || dentro != 0) {
        std::cout << "ERROR: se supero la capacidad del Semaforo propio \n";
        return 1;
    }

    std::cout << "Prueba de concurrencia del Semaforo propio: OK \n"
        return 0;
}

int probarSemaforoEstandar() {
    const int numeroHilos = 12; //Se lanzan 12 hilos
    const int capacidadSemaforo = 3; //Capacidqad 3

    std::counting_semaphore<3> semaforoCapacidad(capacidadSemaforo);
    std::mutex mutexContador;
    int dentro = 0;
    int maximoDentro = 0;
    std::vector<std::thread> hilos;


    for (int i = 0; i < numeroHilos; ++i) {
        hilos.emplace_back([&, i]() {
            tareaConcurrenteEstandar(semaforoCapacidad, i + 1, mutexContador, dentro, maximoDentro);
            });
    }

    for (std::thread& hilo : hilos) {
        hilo.join();
    }

    std::cout << "Maximo de hilos dentro: " << maximoDentro << " de " << capacidadSemaforo << "\n";

    if (maximoDentro > capacidadSemaforo || dentro != 0) {
        std::cout << "ERROR: se supero la capacidad del Semaforo estandar \n";
        return 1;
    }

    std::cout << "Prueba de concurrencia del Semaforo estandar: OK \n";
        return 0;
}

int comprobarTimeoutCero() {

    //Inicializo Semaforo en 0 para forzar que expire el tiempo de espera
    Semaforo semaforoCero(0);

    //Pongo de tiempo de espera como maximo 50 ms a poder adquirir el semaforo
    const bool adquirido = semaforoCero.tryAcquireFor(std::chrono::milliseconds(50));

    //Como esta en 0, debe expirar, devolver false, e imprimir OM
    if (!adquirido) {
        std::cout << "OK\n";
        return 0;
    }
    else {
        std::cout << "ERROR: tryAcquireFor no deberia adquirir con valor 0\n";
        return 1;
    }
}

int comprobarTimeoutCeroEstandar() {

    //Inicializo Semaforo en 0 para forzar que expire el tiempo de espera
    std::counting_semaphore<3> semaforoCero(0);

    //Pongo de tiempo de espera como maximo 50 ms a poder adquirir el semaforo
    const bool adquirido = semaforoCero.tryAcquireFor(std::chrono::milliseconds(50));

    //Como esta en 0, debe expirar, devolver false, e imprimir OM
    if (!adquirido) {
        std::cout << "OK\n";
        return 0;
    }
    else {
        std::cout << "ERROR: tryAcquireFor no deberia adquirir con valor 0\n";
        return 1;
    }
}

int main() {
    std::cout << "=== INICIO DE LA PRUEBA DE SEMÁFORO (12 hilos, Módulo 3) ===\n";

    const int numeroHilos = 12; //Se lanzan 12 hilos
    const int capacidadSemaforo = 3; //Capacidqad 3
    Semaforo semaforoCapacidad(capacidadSemaforo);

    std::vector<std::thread> hilos;

    // Lanzamos 12 hilos concurrentes pasandoles el semaforo por referencia
    for (int i = 0; i < numeroHilos; ++i) {
        hilos.emplace_back(tareaConcurrente, std::ref(semaforoCapacidad), i + 1);
    }

    // Esperamos a que todos los hilos terminen de ejecutarse antes de cerrar el programa
    for (auto& t : hilos) {
        t.join();
    }

    //Comprobamos tryAcquireFor(50 ms) con un semaforo en 0
    comprobarTimeoutCero();

    std::cout << "=== FIN DE PRUEBA SEMAFORO ===\n";
    return 0;
}