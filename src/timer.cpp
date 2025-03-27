#include "timer.h"

Timer::Timer() {
    start();
}

void Timer::start() {
    startTime = std::chrono::high_resolution_clock::now();
}

double Timer::elapsedSeconds() const {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - startTime;
    return elapsed.count();
}

AggregateTimer::AggregateTimer() : totalSeconds(0.0) {}

void AggregateTimer::addDuration(double seconds) {
    totalSeconds += seconds;
}

double AggregateTimer::totalElapsed() const {
    return totalSeconds;
}

void AggregateTimer::reset() {
    totalSeconds = 0.0;
}
