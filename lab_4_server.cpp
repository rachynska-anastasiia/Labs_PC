#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <conio.h>
#define MY_PORT 888

using namespace std;

void task(vector<int> &result_matrix, vector<int> &matrix, int N, int start, int end) {
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++)
            result_matrix[j * N + i] = matrix[i * N + j];
    }
}

void printMatrix(vector<int> matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << setw(4) << matrix[i * N + j] << " ";
        }
        cout << endl;
    }
}

void work_with_client(SOCKET socket) {
    vector<int> result_matrix;
    vector<int> matrix;
    char tag, error_tag = 0x09, not_started_teg = 0x07, in_process = 0x08;
    int N, temp, threads_number, counter, size, i;
    bool has_size = false, has_threads = false, has_numbers = false, processing = false, done = false;
    unsigned long mode = 0;
    ioctlsocket(socket, FIONBIO, &mode);

    while (recv(socket, &tag, 1, 0) > 0) {
        //читаємо розмірність матриці
        if (tag == 0x01) {
            recv(socket, (char *) &temp, 4, 0);
            N = ntohl(temp);
            cout << "N = " << N << endl;
            matrix.resize(N * N);
            result_matrix.resize(N * N);
            has_size = true;
        }

        //читаємо кількість потоків для виконання задачі
        else if (tag == 0x02) {
            recv(socket, (char *) &temp, 4, 0);
            threads_number = ntohl(temp);
            cout << "theads_number = " << threads_number << endl;
            has_threads = true;
        }

        //отримуємо матрицю
        else if (tag == 0x03) {
            recv(socket, (char *) &counter, 4, 0);
            counter = ntohl(counter);
            if (counter != N * N || counter == 0) {
                vector<char> junk(counter * 4);
                if (counter > 0) recv(socket, junk.data(), counter * 4, 0);
                send(socket, (char *) &error_tag, 1, 0);
            } else {
                for (i = 0; i < counter; i++) {
                    recv(socket, (char *) &temp, 4, 0);
                    matrix[i] = ntohl(temp);
                }
                has_numbers = true;
                printMatrix(matrix, N);
                send(socket, (char *) &tag, 1, 0);
            }
        }

        //запускаємо обчислення
        else if (tag == 0x04) {
            if (!has_numbers || !has_size || !has_threads)
                send(socket, &error_tag, 1, 0);
            else {
                send(socket, &tag, 1, 0);

                vector<thread> threads;
                size = N / threads_number;

                processing = true;
                for (i = 0; i < threads_number - 1; i++)
                    threads.emplace_back(task, ref(result_matrix), ref(matrix), N, i * size, (i + 1) * size);
                threads.emplace_back(task, ref(result_matrix), ref(matrix), N, i * size, N);

                for (i = 0; i < threads_number; i++) threads[i].join();
                done = true;
                processing = false;
                send(socket, &tag, 1, 0);
            }
        }

        //надсилаємо результат
        else if (tag == 0x05) {
            if (!has_numbers || !has_size || !has_threads)
                send(socket, &error_tag, 1, 0);
            else if (done) {
                send(socket, &tag, 1, 0);
                for (i = 0; i < N * N; i++) {
                    temp = htonl(result_matrix[i]);
                    send(socket, (char *) &temp, 4, 0);
                }
            } else if (processing) send(socket, &in_process, 1, 0);
            else send(socket, &not_started_teg, 1, 0);
        } else if (tag == 0x06) {
            closesocket(socket);
            return;
        }
    }
}

int main() {
    cout << "TCP SERVER \n";

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
   
    SOCKET my_socket;
    if ((my_socket = socket(AF_INET,SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cout << "Error socket " << WSAGetLastError();
        WSACleanup();
        return -1;
    }

    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MY_PORT);
    local_addr.sin_addr.s_addr = 0; //слухаємо всі ір //INADDR_ANY

    if (bind(my_socket, (sockaddr *) &local_addr, sizeof(local_addr))) {
        cout << "Error bind %d\n" << WSAGetLastError();
        closesocket(my_socket);
        WSACleanup();
        return -1;
    }

    cout << "Waiting for connecting..." << endl;
    cout << "Enter q to exit" << endl;

    if (listen(my_socket, 0x100)) {
        cout << "Error listen %d\n" << WSAGetLastError();
        closesocket(my_socket);
        WSACleanup();
        return -1;
    }

    unsigned long mode = 1;
    ioctlsocket(my_socket, FIONBIO, &mode);
    while (true) {
        if (_kbhit())
            if (_getch() == 'q') break;

        SOCKET client_socket = accept(my_socket, NULL, NULL);
        if (client_socket != INVALID_SOCKET)
            thread(work_with_client, client_socket).detach();
        else Sleep(10);
    }

    closesocket(my_socket);
    WSACleanup();
    return 0;
}
