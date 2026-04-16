#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>

#define PORT 666
#define SERVERADDR "127.0.0.1"

using namespace std;

void printMatrix(vector<int> matrix, int total_size_matrix) {
    for (int i = 0; i < total_size_matrix; i++) {
        for (int j = 0; j < total_size_matrix; j++) {
            cout << setw(4) << matrix[i*total_size_matrix+j] << " ";
        }
        cout << endl;
    }
}

void generateMatrix(vector<int>& matrix, int N) {
    srand(time(NULL));
    for (int i = 0; i < N; i++)
        matrix[i] = rand()%100 +1;
}

int main() {
    printf("TCP CLIENT\n");
    vector<char> packet(5);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

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

    if (connect(my_socket, (struct sockaddr *) &dest_addr, sizeof(dest_addr))) {
        cout << "Connect error " << WSAGetLastError();
        return -1;
    }

    int N, threads_num, total_size_matrix, temp, result;
    cout << "Enter number of rows and columns: ";
    cin >> N;
    total_size_matrix = N*N;
    temp = htonl(N);
    packet[0]=0x01;
    memcpy(&packet[1], &temp, sizeof(int));
    result = send(my_socket, packet.data(), 5, 0);

    cout << "Enter number of threads: ";
    cin >> threads_num;
    temp = htonl(threads_num);
    packet[0]=0x02;
    memcpy(&packet[1], &temp, sizeof(int));
    result = send(my_socket, packet.data(), 5, 0);

    cout << total_size_matrix << endl;
    vector matrix(total_size_matrix, 0);
    vector result_matrix(total_size_matrix, 0);
    generateMatrix(matrix, total_size_matrix);
    printMatrix(matrix, N);

    int packet_size = total_size_matrix*sizeof(int)+5;
    if (packet.size() < packet_size) {
        packet.resize(packet_size);
    }

    packet[0]=0x03;
    int counter = htonl(total_size_matrix);
    memcpy(&packet[1], &counter, sizeof(int));
    for (size_t i = 0; i < total_size_matrix; i++) {
        temp = htonl(matrix[i]);
        memcpy(&packet[5+i*sizeof(int)], &temp, sizeof(int));
    }
    send(my_socket, packet.data(), packet_size, 0);

    //packet.resize(1);
    packet[0]=0x04;
    send(my_socket, packet.data(), 1, 0);
    char response;
    recv(my_socket, &response, 1, 0);
    cout << response << endl;

    packet[0]=0x05;
    send(my_socket, packet.data(), 1, 0);
    recv(my_socket, &response, 1, 0);

    if (response == 0x05) {
        for (int i = 0; i < total_size_matrix; i++) {
            recv(my_socket, (char*)&temp, 4, 0);
            result_matrix[i]= ntohl(temp);
        }
        printMatrix(result_matrix, N);
    }

    packet[0]=0x06;
    send(my_socket, packet.data(), 1, 0);

    closesocket(my_socket);
    WSACleanup();
    return 0;
}

/*надсилаємо значення серверу та отримуємо відповідь про успішність відправлення
    LARGE_INTEGER frequency, start_time, end_time;
QueryPerformanceFrequency(&frequency);
QueryPerformanceCounter(&start_time);

QueryPerformanceCounter(&end_time);
double duration = (double) (end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
*/