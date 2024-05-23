#include <iostream>
#include <fstream>
#include <cmath>
#include "timer.h"
#include <thread>
#include <future>
#include <vector>

#ifndef CPP_LAB_15_16_MATRIX_H
#define CPP_LAB_15_16_MATRIX_H

std::mutex mtx;
unsigned int cores = std::thread::hardware_concurrency();

template <typename T>
class matrix {

private:
    int row;
    int col;
    T ** mat;

public:
    int getRow() const {
        return row;
    }

    int getCol() const {
        return col;
    }

    T **getMat() const {
        return mat;
    }

private:
    void setRow(int rowV) {
        row = rowV;
    }

    void setCol(int colV){
        col = colV;
    }

    void setMat(T** matV) {
        mat = matV;
    }

public:
    //получить значение из данной ячейки матрицы
    T getValue(int i, int j) const{
        return mat[i][j];
    }

    //поменять значение в данной ячейке матрицы
    void setValue(int i, int j, T value){
        mat[i][j] = value;
    }

    //конструктор пустой матрицы заданного размера
    matrix(int rowValue, int colValue){
        row = rowValue;
        col = colValue;
        mat = new T* [row];
        for (int i = 0; i < row; i++){
            mat[i] = new T [col];
        }
    }

    //конструктор для считывания матрицы с консоли
    matrix(){
        std::cout<<"Введите размеры матрицы"<<std::endl;
        int rowV;
        int colV;
        std::cin>>rowV;
        std::cin>>colV;
        row = rowV;
        col = colV;
        mat = new T* [row];
        for (int i = 0; i < row; i++){
            mat[i] = new T [col];
        }
        T value;
        std::cout<<"Введите элементы матрицы"<<std::endl;
        for (int i = 0; i < row; i++){
            for (int j = 0; j < col; j++){
                std::cin>>value;
                mat[i][j] = value;
            }
        }
    }

    //конструктор для считывния матрицы из файла
    explicit matrix(const std::string& path) {

        std::ifstream fin;
        fin.open(path);
        int rowValue, colValue;
        fin>>rowValue;
        fin>>colValue;
        row = rowValue;
        col = colValue;
        mat = new T* [row];
        for (int i = 0; i < row; i++){
            mat[i] = new T [col];
        }
        for (int i = 0; i < row; ++i){
            for (int j = 0; j < col; ++j){
                T value;
                fin>>value;
                mat[i][j] = value;
            }
        }
        fin.close();
    }

    //считывание элементов матрицы из консоли
    void set(){
        std::cout<<"Введите элементы матрицы"<<std::endl;
        T value;
        for (int i = 0; i < row; ++i){
            for (int j = 0; j < col; ++j){
                std::cin>>value;
                mat[i][j] = value;
            }
        }
    }

    //конструктор копирования
    matrix(const matrix &other){
        row = other.getRow();
        col = other.getCol();
        mat = new T *[other.getRow()];
        for (int i = 0; i < other.getRow(); i++){
            mat[i] = new T [other.getCol()];
        }
        for (int i = 0; i < other.row; ++i){
            for (int j = 0; j < other.col; ++j){
                mat[i][j] = other.getMat()[i][j];
            }
        }
    }

//вспомогательный метод для перегрузки оператора присваивания
private:
    void swap(matrix& m){

        int tmpRow = row;
        row = m.getRow();
        m.setRow(tmpRow);

        int tmpCol = col;
        col = m.getCol();
        m.setCol(tmpCol);

        T ** tmpMat = mat;
        mat = m.getMat();
        m.setMat(tmpMat);
    }

public:
    //перегрузка оператора присваивания
    matrix& operator = (const matrix& A){
        matrix tmp(A);
        swap(tmp);
        return *this;
    }

    //перегрузка оператора << для вывода матрицы в консоль
    friend std::ostream& operator<<(std::ostream &out, const matrix<T> matrix){

        std::lock_guard<std::mutex> guard(mtx);

        for (int i = 0; i < matrix.row; i++){
            for (int j = 0; j < matrix.col; j++){
                out<<matrix.mat[i][j]<<' ';
            }
            out<<std::endl;
        }
        return out;
    }

    //вывод матрицы в файл
    static void printToFile (matrix& A, const std::string& path){

        std::lock_guard<std::mutex> guard(mtx);

        std::ofstream fout;
        fout.open(path);
        for (int i = 0; i < A.row; i++){
            for (int j = 0; j < A.col; ++j){
                fout<<A.mat[i][j]<<' ';
            }
            fout<<std::endl;
        }
        fout.close();
    }

    //перегрузка оператора + для сложения матриц
    friend matrix operator + (const matrix& A, const matrix& B){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);
            for (int i = 0; i < rowC; ++i){
                for (int j = 0; j < colC; ++j){
                    C.mat[i][j] = A.mat[i][j] + B.mat[i][j];
                }
            }
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить сумму. Размер матриц не совпадает");
        }
    }

    static matrix<T> parallelSum(const matrix<T>& A, const matrix<T>& B){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::thread> threads;

            unsigned int maxThreads;
            if (A.getRow() <= cores)
                maxThreads = A.getRow();
            else
                maxThreads = cores;
            unsigned int forOneThread = A.getRow() / maxThreads;

            for (int i = 0; i < maxThreads; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == maxThreads-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                threads.emplace_back([begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            C.setValue(j, k, A.getValue(j, k)+B.getValue(j, k));
                        }
                    }
                });
            }

            std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить сумму. Размер матриц не совпадает");
        }
    }

    static matrix<T> parallelSum(const matrix<T>& A, const matrix<T>& B, unsigned int blockSize){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::future<void>> futures;

            if (A.getRow() < blockSize){
                blockSize = A.getRow();
            }

            unsigned int forOneThread = A.getRow() / blockSize;

            for (int i = 0; i < blockSize; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == blockSize-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                futures.emplace_back(std::async(std::launch::async, [begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            C.setValue(j, k, A.getValue(j, k)+B.getValue(j, k));
                        }
                    }
                }));
            }

            std::for_each(futures.begin(),futures.end(),std::mem_fn(&std::future<void>::wait));
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить сумму. Размер матриц не совпадает");
        }
    }

    //перегрузка оператора - для вычитания матриц
    friend matrix operator - (matrix& A, matrix& B){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);
            for (int i = 0; i < rowC; ++i){
                for (int j = 0; j < colC; ++j){
                    C.mat[i][j] = A.mat[i][j] - B.mat[i][j];
                }
            }
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить разность. Размер матриц не совпадает");
        }
    }

    static matrix<T> parallelSubtraction(const matrix<T>& A, const matrix<T>& B){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::thread> threads;

            unsigned int maxThreads;
            if (A.getRow() <= cores)
                maxThreads = A.getRow();
            else
                maxThreads = cores;
            unsigned int forOneThread = A.getRow() / maxThreads;

            for (int i = 0; i < maxThreads; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == maxThreads-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                threads.emplace_back([begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            C.setValue(j, k, A.getValue(j, k)-B.getValue(j, k));
                        }
                    }
                });
            }

            std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить разность. Размер матриц не совпадает");
        }
    }

    static matrix<T> parallelSubtraction(const matrix<T>& A, const matrix<T>& B, unsigned int blockSize){
        if (A.getRow() == B.getRow() && A.getCol() == B.getCol()){
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::future<void>> futures;

            if (A.getRow() < blockSize){
                blockSize = A.getRow();
            }
            unsigned int forOneThread = A.getRow() / blockSize;

            for (int i = 0; i < blockSize; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == blockSize-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                futures.push_back(std::async(std::launch::async, [begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            C.setValue(j, k, A.getValue(j, k)-B.getValue(j, k));
                        }
                    }
                }));
            }

            std::for_each(futures.begin(),futures.end(),std::mem_fn(&std::future<void>::wait));
            return C;
        }
        else{
            throw std::invalid_argument("Невозможно вычислить сумму. Размер матриц не совпадает");
        }
    }

    //перегрузка оператора * для умножения матриц
    friend matrix operator * (matrix& A, matrix& B){
        if (A.col != B.row){
            throw std::invalid_argument("Данные матрицы нельзя перемножить");
        }
        else{
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);
            for (int i = 0; i < rowC; ++i){
                for (int j = 0; j < colC; ++j){
                    C.mat[i][j] = 0;
                    for (int k = 0; k < A.getCol(); ++k){
                        C.mat[i][j] += A.mat[i][k] * B.mat[k][j];
                    }
                }
            }
            return C;
        }
    }

    static matrix<T> parallelMultiplication(const matrix& A, const matrix& B){
        if (A.col != B.row){
            throw std::invalid_argument("Данные матрицы нельзя перемножить");
        }
        else{
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::thread> threads;

            unsigned int maxThreads;
            if (A.getRow() <= cores)
                maxThreads = A.getRow();
            else
                maxThreads = cores;
            unsigned int forOneThread = A.getRow() / maxThreads;

            for (int i = 0; i < maxThreads; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == maxThreads-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                threads.emplace_back([begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            T elem = 0;
                            for (unsigned int m = 0; m < A.getCol(); m++){
                                elem += (A.getValue(j, m)) * (B.getValue(m, k));
                            }
                            C.setValue(j, k, elem);
                        }
                    }
                });
            }

            std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));
            return C;
        }
    }

    static matrix<T> parallelMultiplication(const matrix& A, const matrix& B, unsigned int blockSize){
        if (A.col != B.row){
            throw std::invalid_argument("Данные матрицы нельзя перемножить");
        }
        else{
            timer t;
            int rowC = A.getRow();
            int colC = B.getCol();
            matrix C(rowC, colC);

            std::vector<std::future<void>> futures;

            if (A.getRow() < blockSize){
                blockSize = A.getRow();
            }
            unsigned int forOneThread = A.getRow() / blockSize;

            for (int i = 0; i < blockSize; i++){

                unsigned int begin = forOneThread * i;
                unsigned int end;
                if (i == blockSize-1)
                    end = A.getRow();
                else
                    end = begin + forOneThread;

                futures.push_back(std::async(std::launch::async, [begin, end, &A, &B, &C] () mutable {
                    for (unsigned int j = begin; j < end; j++){
                        for (unsigned int k = 0; k < A.getCol(); k++){
                            T elem = 0;
                            for (unsigned int m = 0; m < A.getCol(); m++){
                                elem += (A.getValue(j, m)) * (B.getValue(m, k));
                            }
                            C.setValue(j, k, elem);
                        }
                    }
                }));
            }

            std::for_each(futures.begin(),futures.end(),std::mem_fn(&std::future<void>::wait));
            return C;
        }
    }

    //перегрузка оператора * для умножения матрицы на число
    friend matrix operator * (matrix& A, double x){
        int rowC = A.getRow();
        int colC = A.getCol();
        matrix C(rowC, colC);
        for (int i = 0; i < rowC; ++i){
            for (int j = 0; j < colC; ++j){
                C.mat[i][j] = A.mat[i][j] * x;
            }
        }
        return C;
    }

    //перегрузка оператора == для сравнения матриц
    friend bool operator == (matrix& A, matrix& B){
        if (A.getCol() != B.getCol() || A.getRow() != B.getRow()){
            return false;
        }
        std::lock_guard<std::mutex> guard(mtx);
        for (int i = 0; i < A.getRow(); ++i){
            for (int j = 0; j < A.getCol(); ++j){
                if (A.mat[i][j] != B.mat[i][j]){
                    return false;
                }
            }
        }
        return true;
    }

    //перегрузка оператора != для сравнения матриц
    friend bool operator != (matrix& A, matrix& B){
        if (A.getCol() != B.getCol() || A.getRow() != B.getRow()){
            return true;
        }
        std::lock_guard<std::mutex> guard(mtx);
        for (int i = 0; i < A.getRow(); ++i){
            for (int j = 0; j < A.getCol(); ++j){
                if (A.mat[i][j] != B.mat[i][j]){
                    return true;
                }
            }
        }
        return false;
    }

    //перегрузка оператора == для сравнения матрицы и скаляра
    friend bool operator == (matrix& A, T x){
        bool flag = true;
        for (int i = 0; i < A.getRow(); ++i){
            for (int j = 0; j < A.getCol(); ++j){
                if (i == j){
                    if (A.mat[i][j] != x){
                        flag = false;
                    }
                }
                else{
                    if (A.mat[i][j] != 0){
                        flag = false;
                    }
                }
            }
        }
        return flag;
    }

    //перегрузка оператора != для сравнения матрицы и скаляра
    friend bool operator != (matrix& A, T x){
        bool flag = true;
        for (int i = 0; i < A.getRow(); ++i){
            for (int j = 0; j < A.getCol(); ++j){
                if (i == j){
                    if (A.mat[i][j] != x){
                        flag = false;
                    }
                }
                else{
                    if (A.mat[i][j] != 0){
                        flag = false;
                    }
                }
            }
        }
        return !flag;
    }

    //функция транспонирования матрицы
    static matrix transpose(matrix& A){
        std::lock_guard<std::mutex> guard(mtx);
        matrix transposedA(A.getCol(), A.getRow());
        for (int i = 0; i < A.getRow(); i++){
            for (int j = 0; j < A.getCol(); j++){
                transposedA.getMat()[j][i] = A.getMat()[i][j];
            }
        }
        return transposedA;
    }

private:
    //функция получения матрицы без i-ой строки и j-ого столбца
    static matrix getMatrixWithoutRowCol(matrix& A, int size, int i_row, int j_col){
        matrix newMat(size-1, size-1);
        int rowShift = 0;
        int colShift;
        for (int i = 0; i < size-1; ++i){
            if (i == i_row){
                rowShift = 1;
            }
            colShift = 0;
            for (int j = 0; j < size-1; ++j){
                if (j == j_col){
                    colShift = 1;
                }
                newMat.getMat()[i][j] = A.getMat()[i+rowShift][j+colShift];
            }
        }
        return newMat;
    }

public:

    //функция вычисления определителя матрицы
    static T determinant (matrix& A){
        if (A.getCol() != A.getRow()){
            throw std::invalid_argument("Определитель не существует");
        }
        else {
            timer t;
            std::recursive_mutex recursiveMutex;
            recursiveMutex.lock();
            int size = A.getRow();
            T det = 0;

            if (size == 1){
                recursiveMutex.unlock();
                return A.getMat()[0][0];
            }
            else if (size == 2){
                recursiveMutex.unlock();
                return A.getMat()[0][0] * A.getMat()[1][1] - A.getMat()[0][1] * A.getMat()[1][0];
            }
            else {
                for (int i = 0; i < size; ++i){
                    for (int j = 0; j < size; ++j){
                        if (i == 0){
                            matrix newA(matrix::getMatrixWithoutRowCol(A, A.getRow(), 0, j));
                            det = det + (pow(-1, (i + j)) * A.getMat()[i][j] * determinant(newA));
                        }
                    }
                }
                recursiveMutex.unlock();
                return det;
            }
        }
    }

    //перегрузка оператора ! для нахождения обратной матрицы
    friend matrix operator ! (matrix& A){
        T det = matrix<T>::determinant(A);
        if (det == 0){
            throw std::invalid_argument("Обратная матрица не существует");
        }
        else {
            double x = 1/det;
            matrix transposedA(matrix<T>::transpose(A));
            matrix inverseA(A.getRow(), A.getCol());
            for (int i = 0; i < A.getRow(); ++i){
                for (int j = 0; j < A.getCol(); ++j){
                    matrix Mij(matrix::getMatrixWithoutRowCol(transposedA, transposedA.getRow(), i, j));
                    inverseA.mat[i][j] = pow(-1, (i+j)) * matrix::determinant(Mij) * x;
                }
            }
            return inverseA;
        }
    }

    //метод для создания нулевой матрицы заданного размера
    static matrix createZeroMatrix(int rowV, int colV){
        matrix O(rowV, colV);
        T** matV;
        matV = new T* [rowV];
        for (int i = 0; i < rowV; i++){
            matV[i] = new T [colV];
        }
        for (int i = 0; i < rowV; i++){
            for (int j = 0; j < colV; j++){
                matV[i][j] = 0;
            }
        }
        O.setMat(matV);
        return O;
    }

    //метод для создания единичной матрицы заданного размера
    static matrix createIdentityMatrix(int rowV, int colV){
        matrix E(rowV, colV);
        T ** matV;
        matV = new T* [rowV];
        for (int i = 0; i < rowV; i++){
            matV[i] = new T [colV];
        }
        for (int i = 0; i < rowV; i++){
            for (int j = 0; j < colV; j++){
                if (i == j){
                    matV[i][j] = 1;
                } else{
                    matV[i][j] = 0;
                }
            }
        }
        E.setMat(matV);
        return E;
    }

    //деструктор
    ~matrix(){
        delete[] mat;
    }

};


#endif //CPP_LAB_15_16_MATRIX_H
