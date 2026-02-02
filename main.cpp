#include <iostream>
#include <ctime>
#include <vector>
#include <chrono>

using namespace std;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;


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
            cout << matrix[i][j] << " ";
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


    int n;
    cout << "Enter size n of matrix n*n = ";
    cin >> n;

    vector matrix(n,vector<int>(n));
    cout << "Matrix elements" << endl;
    generateMatrix(matrix, n);

    //printMatrix(matrix, n);

    auto start = high_resolution_clock::now();

    transpose(matrix, n);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);
    cout << duration.count() << endl;//*1e-9

    //cout << "Results:" << endl;
    //printMatrix(matrix, n);
    return 0;
}