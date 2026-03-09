#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

const int N = 10000;
int number[N];

//функція розрахунків
void calculations(long long& sum, int& min) {
    for (int i = 0; i < N; i++) {
        if (number[i] % 2) {
            sum += number[i];
            if (number[i]<min) min = number[i];
        }
    }
}

int main() {
    cout << "Simple" << endl;
    //отримання початкових даних про масив
    int left_border = 1, right_border = 99;

    //генерація значень масиву
    for (int i = 0; i < N; i++) {
        number[i] = rand()%(right_border-left_border) + left_border;
    }

    //змінні для пошуку
    long long sum=0;
    int min=right_border+1;

    //заміри часу та обрахунки
    auto start = high_resolution_clock::now();

    calculations(sum, min);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);


    //вивід результатів
    //for (int i = 0; i < n; i++) cout << numbers[i] << " ";
    cout << endl << "Duration = " << duration.count()*1e-09 << endl;
    cout << "Sum = " << sum << endl;
    cout << "Min = " << min << endl;

    return 0;
}