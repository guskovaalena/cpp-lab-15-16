#include "timer.h"

timer::timer() {
//    std::cout<<"Process started."<<std::endl;
    start = std::chrono::high_resolution_clock::now();
}

timer::~timer() {
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
//    std::cout<<"Process ended."<<std::endl;
    std::cout<<"Duration: "<<duration.count()<<" s"<<std::endl;
}