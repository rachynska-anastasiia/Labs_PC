#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

//Виконати транспонування матриці n×n.
const int N=15000;
volatile int matrix[N][N];
volatile int result_matrix_pararell[N][N];

static void task(int beginning,  int end) {
    for (int i = beginning; i < end; i++) {
        for (int j = 0; j < N; j++) {
            result_matrix_pararell[i][j] = matrix[j][i];
        }
    }
}

void printMatrix(volatile int (&matrix)[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout  << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void generateMatrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand()%100 +1;
        }
    }
}

int main() {
    return 0;
}