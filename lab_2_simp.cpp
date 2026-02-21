#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

void calculations(int& sum, int& min, vector<int>& numbers) {
    for (int i = 0; i < numbers.size(); i++) {
        if (numbers[i] % 2) {
            sum += numbers[i];
            if (numbers[i]<min) min = numbers[i];
        }
    }
}

int main() {
    int n, left_border, right_border;

    cout << "Enter size of array: ";
    cin >> n;

    cout << "Enter left border of array: ";
    cin >> left_border;

    right_border = left_border - 1;
    while (left_border > right_border) {
        cout << "Enter right border of array: ";
        cin >> right_border;
    }

    vector<int> numbers(n);
    for (int i = 0; i < n; i++) {
        numbers[i] = rand()%(right_border-left_border) + left_border;
    }

    int sum=0, min=right_border+1;

    auto start = high_resolution_clock::now();

    calculations(sum, min, numbers);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);

    for (int i = 0; i < n; i++) {
        cout << numbers[i] << " ";
    }
    cout << "Duration = " << duration.count()*1e-9 << endl;//*1e-9
    cout << "Sum = " << sum << endl;
    cout << "Min = " << min << endl;

    return 0;
}