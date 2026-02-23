#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

atomic<int> sum(0), minimal(1);

void calculations(vector<int>& numbers, int begin, int end, int& right) {
    int local_sum = 0, global_sum;
    int local_min = right, global_min;
    for (int i = begin; i < end; i++) {
        if (numbers[i] % 2) {
            local_sum += numbers[i];
            if (numbers[i]<local_min) local_min = numbers[i];
        }
    }

    do {
        global_sum = sum.load();
    } while (!sum.compare_exchange_strong(global_sum, local_sum + global_sum));

    do {
        global_min = minimal.load();
        if (global_min <= local_min) break;
    }while (!minimal.compare_exchange_strong(global_min, local_min));
}

int main() {
    int n, left_border, right_border;
    int number_of_threads=16;

    cout << "Enter size of array: ";
    cin >> n;
    cout << "Enter left border of array: ";
    cin >> left_border;
    cout << "Enter right border of array: ";
    cin >> right_border;

    vector<int> numbers(n);
    for (int i = 0; i < n; i++) {
        numbers[i] = rand()%(right_border-left_border) + left_border;
    }
    int global_min = minimal.load();
    minimal.compare_exchange_strong(global_min, global_min + right_border);

    vector<thread> threads;
    int i, size = int(n / number_of_threads);


    auto start = high_resolution_clock::now();

    for (i = 0; i < n; i++) {
        threads.emplace_back(calculations, ref(numbers), i*size, (i+1)*size, ref(right_border));
    }
    threads.emplace_back(calculations, ref(numbers), i*size, n, ref(right_border));

    for (i = 0; i < threads.size(); i++) threads[i].join();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);


    for (int i = 0; i < n; i++) {
        cout << numbers[i] << " ";
    }
    cout << "Duration = " << duration.count()*1e-9 << endl;//*1e-9
    cout << "Sum = " << sum << endl;
    cout << "Min = " << minimal << endl;

    return 0;
}