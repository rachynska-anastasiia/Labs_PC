#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

const int N = 10;
int number_of_threads = 16;
int number[N];

atomic<long long> sum(0);
atomic<int> minimal(0);

void calculations(int begin, int end, int &right) {
    long long local_sum = 0, global_sum;
    int local_min = right, global_min;
    for (int i = begin; i < end; i++) {
        if (number[i] % 2) {
            local_sum += number[i];
            if (number[i] < local_min) local_min = number[i];
        }
    }

    do {
        global_sum = sum.load();
    } while (!sum.compare_exchange_weak(global_sum, local_sum + global_sum));

    do {
        global_min = minimal.load();
        if (global_min <= local_min) break;
    } while (!minimal.compare_exchange_weak(global_min, local_min));
}

int main() {
    cout << "Atomic " << N << endl;
    //отримання початкових даних про масив
    int left_border = 1, right_border = 5;

    //генерація значень масиву
    for (int i = 0; i < N; i++) {
        number[i] = rand() % (right_border - left_border) + left_border;
    }

    //початкове значення для мінімального елемента
    minimal.store(1 + right_border);

    vector<thread> threads;
    int i, size = int(N / number_of_threads);

    //заміри часу та обрахунки
    auto start = high_resolution_clock::now();

    for (i = 0; i < number_of_threads - 1; i++) {
        threads.emplace_back(calculations, i * size, (i + 1) * size, ref(right_border));
    }
    threads.emplace_back(calculations, i * size, N, ref(right_border));
    for (i = 0; i < threads.size(); i++) threads[i].join();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);

    //вивід результатів
    for (int i = 0; i < N; i++) cout << number[i] << " ";
    cout << endl << "Duration = " << duration.count() * 1e-09 << endl;
    cout << "Sum = " << sum << endl;
    cout << "Min = " << minimal << endl;

    return 0;
}