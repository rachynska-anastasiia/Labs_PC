#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

const int N = 10;
int number_of_threads = 16;
int number[N];

long long sum=0;
int minimal;
mutex main_mutex;

void calculations(int begin, int end, int& right) {
    for (int i = begin; i < end; i++) {
        if (number[i] % 2 == 1) {
            lock_guard<mutex> lock(main_mutex);
            sum += number[i];
            if (number[i]<minimal) minimal = number[i];
        }
    }
}

int main() {
    cout << "Mutex " << N <<endl;
    //отримання початкових даних про масив
    int left_border = 1, right_border = 5;

    //генерація значень масиву
    for (int i = 0; i < N; i++) {
        number[i] = rand()%(right_border-left_border) + left_border;
    }

    //початкове значення для мінімального елемента
    minimal = right_border+1;

    thread threads[number_of_threads];
    int i, size = int(N / number_of_threads);

    //заміри часу та обрахунки
    auto start = high_resolution_clock::now();

    for (i = 0; i < number_of_threads-1; i++) {
        threads[i] = thread(calculations,i*size, (i+1)*size, ref(right_border));
    }
    threads[i]=thread(calculations, i*size, N, ref(right_border));
    for (i = 0; i < number_of_threads; i++) threads[i].join();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);

    //вивід результатів
    for (int i = 0; i < N; i++)cout << number[i] << " ";
    cout << endl << "Duration = " << duration.count()*1e-09<< endl;
    cout << "Sum = " << sum << endl;
    cout << "Min = " << minimal << endl;

    return 0;
}