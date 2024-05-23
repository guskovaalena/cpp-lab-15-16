#include <iostream>
#include <chrono>

#ifndef CPP_LAB_15_16_TIMER_H
#define CPP_LAB_15_16_TIMER_H

class timer {

private:
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

public:
    timer();
    ~timer();

};

#endif //CPP_LAB_15_16_TIMER_H
