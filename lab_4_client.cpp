#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>

#define PORT 888
#define SERVERADDR "127.0.0.1"

using namespace std;

void printMatrix(vector<int> matrix, int total_size_matrix) {
    for (int i = 0; i < total_size_matrix; i++) {
        for (int j = 0; j < total_size_matrix; j++) {
            cout << setw(4) << matrix[i * total_size_matrix + j] << " ";
        }
        cout << endl;
    }
}

void generateMatrix(vector<int> &matrix, int N) {
    srand(time(NULL));
    for (int i = 0; i < N; i++)
        matrix[i] = rand() % 100 + 1;
}

void client_process(SOCKET my_socket) {
    char tag, response;
    vector<int> result_matrix;
    vector<int> matrix;
    vector<char> packet(5);
    int N, threads_num, total_size_matrix, temp, packet_size, input_tag;

    while (true) {
        cout << "Enter tag = ";
        cin >> input_tag;
        tag = char(input_tag);

        //надсилаємо рзмір матриці
        if (tag == 0x01) {
            N = 0;
            while (N <= 0) {
                cout << "Enter number of rows and columns: ";
                cin >> N;
            }
            total_size_matrix = N * N;
            temp = htonl(N);
            packet[0] = 0x01;
            memcpy(&packet[1], &temp, sizeof(int));
            if (send(my_socket, packet.data(), 5, 0) == SOCKET_ERROR) {
                cout << "server doesn't work" << endl;
                return;
            }

            //створюємо матриці
            matrix.resize(total_size_matrix, 0);
            result_matrix.resize(total_size_matrix, 0);
            generateMatrix(matrix, total_size_matrix);
            printMatrix(matrix, N);

            //змінюємо розмір пакета
            packet_size = total_size_matrix * sizeof(int) + 5;
            if (packet.size() < packet_size) packet.resize(packet_size);

        //надсилаємо кількість потоків
        } else if (tag == 0x02) {
            threads_num = 0;
            while (threads_num <= 0) {
                cout << "Enter number of threads: ";
                cin >> threads_num;
            }
            temp = htonl(threads_num);
            packet[0] = 0x02;
            memcpy(&packet[1], &temp, sizeof(int));
            if (send(my_socket, packet.data(), 5, 0) == SOCKET_ERROR) {
                cout << "server doesn't work" << endl;
                return;
            }

        //надсилаємо дані
        } else if (tag == 0x03) {
            packet[0] = 0x03;
            int counter = htonl(total_size_matrix);
            memcpy(&packet[1], &counter, sizeof(int));
            for (size_t i = 0; i < total_size_matrix; i++) {
                temp = htonl(matrix[i]);
                memcpy(&packet[5 + i * sizeof(int)], &temp, sizeof(int));
            }
            if (send(my_socket, packet.data(), total_size_matrix * sizeof(int) + 5, 0) == SOCKET_ERROR) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (recv(my_socket, &response, 1, 0) <= 0) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (response == 0x03) cout << "data was sent" << endl;
            else cout << "matrix is empty" << endl;

        //просимо обробити
        } else if (tag == 0x04) {
            packet[0] = 0x04;
            if (send(my_socket, packet.data(), 1, 0) == SOCKET_ERROR) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (recv(my_socket, &response, 1, 0) <= 0) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (response == 0x09) cout << "uninitialized" << endl;
            else if (response == 0x04) {
                if (recv(my_socket, &response, 1, 0)<= 0) {
                    cout << "server doesn't work" << endl;
                    return;
                }
                cout << "request to start was sent" << endl;
            }

        //отримуємо результати
        } else if (tag == 0x05) {
            packet[0] = 0x05;
            if (send(my_socket, packet.data(), 1, 0) == SOCKET_ERROR) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (recv(my_socket, &response, 1, 0)<=0) {
                cout << "server doesn't work" << endl;
                return;
            }
            if (response == 0x05) {
                for (int i = 0; i < total_size_matrix; i++) {
                    if (recv(my_socket, (char *) &temp, 4, 0)<= 0) {
                        cout << "server doesn't work" << endl;
                        return;
                    }
                    result_matrix[i] = ntohl(temp);
                }
                printMatrix(result_matrix, N);
            } else if (response == 0x07) cout << "not started" << endl;
            else if (response == 0x08) cout << "in progress" << endl;
            else if (response == 0x09) cout << "uninitialized" << endl;

        //відключаємося
        } else if (tag == 0x06) {
            packet[0] = 0x06;
            send(my_socket, packet.data(), 1, 0);
            break;
        }
    }
}


int main() {
    cout << "TCP CLIENT\n";

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET my_socket;
    if ((my_socket = socket(AF_INET,SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cout << "Socket() error " << WSAGetLastError();
        return -1;
    }

    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
    if (dest_addr.sin_addr.s_addr == INADDR_NONE) {
        cout << "Invalid IP address" << WSAGetLastError();
        return -1;
    }

    if (connect(my_socket, (sockaddr *) &dest_addr, sizeof(dest_addr))) {
        cout << "Connect error " << WSAGetLastError();
        return -1;
    }

    cout << "TAGS:" << endl;
    cout << "1 - enter number of rows and columns" << endl;
    cout << "2 - enter number of threads" << endl;
    cout << "3 - send matrix" << endl;
    cout << "4 - start transposing" << endl;
    cout << "5 - receive status / results" << endl;
    cout << "6 - exit" << endl << endl;

    client_process(my_socket);

    closesocket(my_socket);
    WSACleanup();
    return 0;
}
