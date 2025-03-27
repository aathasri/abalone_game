// timer.h
#pragma once
#include <chrono>

class Timer {
public:
    Timer();
    void start();
    double elapsedSeconds() const;

private:
    std::chrono::high_resolution_clock::time_point startTime;
};

class AggregateTimer {
public:
    AggregateTimer();
    void addDuration(double seconds);
    double totalElapsed() const;
    void reset();

private:
    double totalSeconds;
};
