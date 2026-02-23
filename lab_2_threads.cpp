#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

int sum=0, minimal;
mutex main_mutex;

void calculations(vector<int>& numbers, int begin, int end, int& right) {
    int local_sum = 0;
    int local_min = right+1;
    for (int i = begin; i < end; i++) {
        if (numbers[i] % 2) {
            local_sum += numbers[i];
            if (numbers[i]<local_min) local_min = numbers[i];
        }
    }

    lock_guard<mutex> lock(main_mutex);
    sum+= local_sum;
    if (local_min < minimal) minimal = local_min;

}

int main() {
    int n, left_border, right_border;

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

    minimal = right_border+1;

    vector<thread> threads;
    int number_of_threads = 16;
    int i, size = int(n / number_of_threads);


    auto start = high_resolution_clock::now();

    for (i = 0; i < n; i++) {
        threads.emplace_back(calculations, ref(numbers),i*size, (i+1)*size, ref(right_border));
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