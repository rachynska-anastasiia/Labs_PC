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
static const int threads_num = 10;

int n=1000;

static void task(int i, vector<vector<int>>& matrix, int end) {
    for (int j=i+1; j<n; j++) {
        int temp = matrix[i][j];
        matrix[i][j] = matrix[j][i];
        matrix[j][i] = temp;
    }
}

void transpose(vector<vector<int>>& matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            int temp = matrix[i][j];
            matrix[i][j] = matrix[j][i];
            matrix[j][i] = temp;
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
    srand(time(NULL));



    //cout << "Enter size n of matrix n*n = ";
    //cin >> n;

    vector matrix(n,vector<int>(n));
    cout << "Matrix elements" << endl;
    generateMatrix(matrix, n);

    //printMatrix(matrix, n);

    auto start = high_resolution_clock::now();

    transpose(matrix, n);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);
    cout << duration.count()*1e-9 << endl;//*1e-9


    cout << "Results:" << endl;
    //printMatrix(matrix, n);

    cout << endl;

    thread threads[threads_num];

    auto new_start = high_resolution_clock::now();
    int size = int(n / threads_num);
    if (size == 0) size = 1;
    int i;

    for (i = 0; i < threads_num-1; i++) {
        threads[i] = thread(task, i, ref(matrix), (i+1)*size-1);
    }
    threads[threads_num-1] = thread(task, i*size, ref(matrix), n-1);

    for (i = 0; i < threads_num; i++) {
        threads[i].join();
    }

    auto new_stop = high_resolution_clock::now();
    auto new_duration = duration_cast<nanoseconds>(new_stop - new_start);
    cout << new_duration.count()*1e-9 << endl;//*1e-9

    cout << "Results:" << endl;
    //printMatrix(matrix, n);

    return 0;
}