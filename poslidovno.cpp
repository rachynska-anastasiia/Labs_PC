#include <iostream>
#include <chrono>

using namespace std;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;


const int N=500;
//створюємо вхідну матрицю та матрицю результат
volatile int matrix[N][N];
volatile int result_matrix_posl[N][N];

//функція транспонування матриці
void transpose(){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            result_matrix_posl[i][j] = matrix[j][i];
        }
    }
}

//друк матриці
void printMatrix(volatile int (&matrix)[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << setw(4) << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

//генерування вмісту матриці
void generateMatrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand()%100 +1;
        }
    }
}

int main() {

    //заповнюємо вхідну матрицю
    generateMatrix();

    //printMatrix(matrix);

    //час початку роботи функції транспонування
    auto start = high_resolution_clock::now();

    transpose();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);
    cout << duration.count()*1e-9 << endl;//*1e-9

    //вивід результатів
    //printMatrix(result_matrix_posl);

    return 0;
}