#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#define MY_PORT 666

using namespace std;

void task(vector<int>& result_matrix, vector<int>& matrix, int N, int start, int end) {
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            result_matrix[j*N+i] = matrix[i*N+j];
        }
    }
}

void printMatrix(vector<int> matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << setw(4) << matrix[i*N+j] << " ";
        }
        cout << endl;
    }
}

void work_with_client(SOCKET socket) {
    vector<int> result_matrix;
    vector<int> matrix;
    char tag;
    int N, temp, threads_number, counter, size, i;

    while (recv(socket, &tag, 1, 0)!=0x06) {
        if (tag == 0x01) {
            recv(socket, (char*)&temp, 4, 0);
            N=ntohl(temp);
            cout << "N = " << N << endl;
        }
        else if (tag == 0x02) {
            recv(socket, (char*)&temp, 4, 0);
            threads_number=ntohl(temp);
            cout << "theads_number = " << threads_number << endl;
        }
        else if (tag == 0x03) {
            recv(socket, (char*)&counter, 4, 0);
            int counter=ntohl(counter);
            matrix.resize(counter);

            for (i = 0; i < counter; i++) {
                recv(socket, (char*)&temp, 4, 0);
                matrix[i] = ntohl(temp);
            }
            printMatrix(matrix, N);
        }
        else if (tag == 0x04) {
            result_matrix.resize(N*N);
            vector<thread> threads;
            size=N/threads_number;
            for (i = 0; i < threads_number-1; i++) {
                threads.emplace_back(task, ref(result_matrix), ref(matrix), N, i*size, (i+1)*size);
            }
            threads.emplace_back(task, ref(result_matrix), ref(matrix), N, i*size, N);
            for (i = 0; i < threads_number; i++) {
                threads[i].join();
            }
            send(socket, &tag, 1, 0);
        }
        else if (tag == 0x05) {
            send(socket, &tag, 1, 0);
            for (i = 0; i < N*N; i++) {
                temp = htonl(result_matrix[i]);
                send(socket, (char*)&temp, 4, 0);
            }
        }
        else if (tag == 0x06)closesocket(socket);
    }
}

int main() {
    printf("TCP SERVER \n");

    //ініціалізація бібліотеки
    WSADATA wsa;
    WSAStartup(0x0202, &wsa);

    //створення сокера для сервера
    SOCKET my_socket;
    if ((my_socket = socket(AF_INET,SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error socket", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // пов'язування сокета з локальним адресом
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MY_PORT);
    local_addr.sin_addr.s_addr = 0; //слухаємо всі ір //INADDR_ANY

    //викликаємо bind для зв'язування
    if (bind(my_socket, (sockaddr *) &local_addr, sizeof(local_addr))) {
        printf("Error bind %d\n", WSAGetLastError());
        closesocket(my_socket);
        WSACleanup();
        return -1;
    }

    // очікування підключення
    printf("Waiting for connecting...\n");
    if (listen(my_socket, 0x100)) {
        printf("Error listen %d\n", WSAGetLastError());
        closesocket(my_socket);
        WSACleanup();
        return -1;
    }

    // клієнтський сокет
    while (true) {
        SOCKET client_socket= accept(my_socket, NULL, NULL);
        if (client_socket != INVALID_SOCKET) {
            thread(work_with_client, client_socket).detach();
        }
    }

    closesocket(my_socket);
    WSACleanup();

    return 0;
}

/*
//початок замірів часу для отримання чисел (а також їх розмірів)
    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);


    QueryPerformanceCounter(&end_time);
    double duration = (double) (end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;

    //підтверджуємо отримання даних
    send(client_socket, "Data received", sizeof("Data received"), 0);

    //друк отриманих значень
    for (int i = 0; i < 2; i++) {
        printf("double_numbers[%d] = %f\n", i, double_numbers[i]);
    }
    for (int i = 0; i < 9; i++) {
        printf("float_numbers[%d] = %f\n", i, float_numbers[i]);
    }

    //Друк результатів по обрахованому часу й розміру
    printf("Elapsed time: %f\n", duration);
    printf("Total number of bytes received: %d\n", num_of_bytes_received);
    if (num_of_bytes_received == num_of_bytes_expected) {
        printf("All the data was received");
    } else {
        printf("Something was missed...");
    }
 */

/*
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    SOCKET client_socket = accept(my_socket, (sockaddr *) &client_addr, &client_addr_size);

    //отримуємо очікуваний розмір даних
    temp = recv(client_socket, (char *) &num_of_bytes_expected, sizeof(int), 0);
    if (temp <= 0) {
        printf("Failed to get expected size\n");
        closesocket(client_socket);
        closesocket(my_socket);
        WSACleanup();
        return -1;
    }
    printf("Expected to receive: %d bytes\n", num_of_bytes_expected);

   */