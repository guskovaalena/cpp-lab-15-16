#include <iostream>
#include "matrix.h"
#include "timer.h"
#include <thread>
#include <random>

int main() {

    setlocale(LC_ALL, "ru");

    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<int> d(1, 20);

    matrix<double> A(100, 100);
    matrix<double> B(100, 100);

    for (int i = 0 ; i < A.getRow(); i++){
        for (int j = 0; j < A.getCol(); j++){
            A.setValue(i, j, d(e));
        }
    }

    for (int i = 0 ; i < B.getRow(); i++){
        for (int j = 0; j < B.getCol(); j++){
            B.setValue(i, j, d(e));
        }
    }


    try {
        timer t;
        for (int i = 0; i < 3; i++) {
            matrix<double> C = A * B;
        }
//    for (int i = 0; i < 3; i++){
//        matrix<double> C_parallel = matrix<double>::parallelMultiplication(A, B);
//    }

        std::cout << std::endl;
    }   catch (const std::invalid_argument& e){
        std::cout<<e.what()<<std::endl;
    }



    return 0;
}
