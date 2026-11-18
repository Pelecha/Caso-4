#include "Semaforo.h"
#include<mutex>
#include<condition_variable>
#include<chrono>

// TODO: implementar. Ahora mismo compila, pero no sincroniza nada.
Semaforo::Semaforo(int valorInicial) : _valor(valorInicial) {  //constructor que inicializa el valor del semáforo con el valor pasado por parámetro
	// TODO
}

void Semaforo::acquire() { //esperar
	std::unique_lock<std::mutex> lck(_m);  //bloquea el mutex para proteger el acceso a _valor
	_cv.wait(lck, [this]() { return _valor > 0; }); //llama cv.wait si el contador _valor es<=0 el hilo se duerme y libera el mutex para q otros hilos puedan trabajar se despierta hasta que _valor sea mayor que 0
	_valor--;  //en cuanto se despierta resta uno al contador
}

void Semaforo::release() { //liberar
	std::unique_lock <std::mutex> lck(_m);  //bloquea el mutex con unique lock
	_valor++;
	_cv.notify_one();  //despierta a un hilo que estaba dormido en el aquire
		// TODO
}

bool Semaforo::tryAcquireFor(std::chrono::milliseconds plazo) {   //adquirir con tiempo limite
	std::unique_lock<std::mutex> lck(_m);
	if (!_cv.wait_for(lck, plazo, [this]() { return _valor > 0; })) { //usa wait_for con el plazo de tiempo determinado si pasa el tiempo y contador esta en 0 devuelve false, es que ha expirado el plazo
		return false;
	}
	_valor--;
return true;
}
