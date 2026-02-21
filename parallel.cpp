#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::thread;

static const int threads_num =352;
const int N=15000;
//створюємо вхідну матрицю та матрицю результат
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
            cout << setw(4) << matrix[i][j] << " ";
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
    generateMatrix();
    //printMatrix(matrix);

    thread threads[threads_num];

    int size, i;
    size = int(N / threads_num);

    auto new_start = high_resolution_clock::now();

    for (i = 0; i < threads_num-1; i++) {
        threads[i] = thread(task, i*size, (i+1)*size);
    }
    threads[threads_num-1] = thread(task, i*size, N);



    for (i = 0; i < threads_num; i++) {
        threads[i].join();
    }

    auto new_stop = high_resolution_clock::now();
    auto new_duration = duration_cast<nanoseconds>(new_stop - new_start);
    cout << new_duration.count()*1e-9 << endl;//*1e-9

    //printMatrix(result_matrix_pararell);

    return 0;
}