#include "Semaforo.h"
#include <mutex>
#include <condition_variable>
#include <chrono>

Semaforo::Semaforo(int valorInicial) : _valor(valorInicial) {
}

void Semaforo::acquire() {
	std::unique_lock<std::mutex> lck(_m);
	_cv.wait(lck, [this]() { return _valor > 0; });
	_valor--;
}

void Semaforo::release() {
	std::unique_lock<std::mutex> lck(_m);
	_valor++;
	_cv.notify_one();
}

bool Semaforo::tryAcquireFor(std::chrono::milliseconds plazo) {
	std::unique_lock<std::mutex> lck(_m);
	if (!_cv.wait_for(lck, plazo, [this]() { return _valor > 0; })) {
		return false;
	}
	_valor--;
	return true;
}
