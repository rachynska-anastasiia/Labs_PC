#include <iostream>
#include <ctime>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::thread;


//16 cernels in cpu
static const int threads_num = 16;
int n=1000;

static void task(int beginning, vector<vector<int>>& matrix, vector<vector<int>>& result_matrix_pararell, int end) {
    for (int i = beginning; i < end; i++) {
        for (int j = 0; j < n; j++) {
            result_matrix_pararell[j][i] = matrix[i][j];
        }
    }
}

void transpose(vector<vector<int>>& matrix, int n, vector<vector<int>>& result_matrix_posl) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result_matrix_posl[i][j] = matrix[j][i];
        }
    }
}

void printMatrix(vector<vector<int>>& matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << setw(4) << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void generateMatrix(vector<vector<int>>& matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand()%100 +1;
        }
    }
}

int main() {
    vector matrix(n,vector<int>(n));
    vector result_matrix_posl(n,vector<int>(n));
    vector result_matrix_pararell(n,vector<int>(n));

    cout << "Matrix elements" << endl;
    generateMatrix(matrix, n);

    //printMatrix(matrix, n);

    auto start = high_resolution_clock::now();

    transpose(matrix, n, result_matrix_posl);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);
    cout << duration.count()*1e-9 << endl;//*1e-9


    cout << "Results:" << endl;
    //printMatrix(result_matrix_posl, n);

    cout << endl;

    thread threads[threads_num];

    auto new_start = high_resolution_clock::now();

    int size, i;
    if (n < threads_num) size = 1;
    else size = int(n / threads_num);

    //cout << size << endl;

    for (i = 0; i < threads_num-1; i++) {
        threads[i] = thread(task, i*size, ref(matrix), ref(result_matrix_pararell), (i+1)*size);
    }
    threads[threads_num-1] = thread(task, i*size, ref(matrix), ref(result_matrix_pararell), n);

    for (i = 0; i < threads_num; i++) {
        threads[i].join();
    }

    auto new_stop = high_resolution_clock::now();
    auto new_duration = duration_cast<nanoseconds>(new_stop - new_start);
    cout << new_duration.count()*1e-9 << endl;//*1e-9

    cout << "Results:" << endl;
    //printMatrix(result_matrix_pararell, n);

    return 0;
}