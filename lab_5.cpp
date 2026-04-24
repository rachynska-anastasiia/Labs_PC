#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#define PORT 8080

using namespace std;

string readFile(string fileName) {
    ifstream file(fileName);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void handleRequest(SOCKET clientSocket) {
    string request, method, path, content, httpResponse;

    request.resize(1024);
    recv(clientSocket, &request[0], request.size(), 0);
    cout << request << endl;

    istringstream iss(request);
    iss >> method >> path;
    string status = "HTTP/1.1 200 OK";
    if (path == "/index.html" || path=="/") content = readFile("index.html");
    else if (path == "/page2.html") content = readFile("page2.html");
    else {
        content = "Unfortunately Not Found 404 \nNo RaAaR :(";
        status = "HTTP/1.1 404 Not Found";
    }

    httpResponse = status + "\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: " + to_string(content.length()) + "\r\nConnection: close\r\n\r\n" + content;

    cout << httpResponse << endl;
    send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);

    closesocket(clientSocket);
}

class ThreadPool {
private:
    vector<thread> threads;
    queue<SOCKET> work_queue;
    mutex mtx;
    condition_variable cv;
    atomic<bool> is_working;
public:
    ThreadPool(int numThreads) {
        is_working = true;
        for (int i = 0; i < numThreads; i++) threads.push_back(thread(worker, this));
    }

    static void worker(ThreadPool* pool) {
        while (true) {
            SOCKET clientSocket;
            {
                unique_lock<mutex> lock(pool->mtx);
                while (pool->is_working && pool->work_queue.empty()) {
                    pool->cv.wait(lock);
                }
                if (!pool->is_working) return;
                clientSocket = pool->work_queue.front();
                pool->work_queue.pop();
            }

            handleRequest(clientSocket);
        }
    }

    void terminate() {
        is_working = false;
        cv.notify_all();
        for (int i = 0; i < threads.size(); i++) threads[i].join();
    }

    void addTask(SOCKET socket) {
        bool added = false;
        {
            unique_lock<mutex> lock(mtx);
            if (work_queue.size()<200) {
                work_queue.push(socket);
                added = true;
            }
        }
        if (added) cv.notify_one();
        else closesocket(socket);
    }

    ~ThreadPool() {
        terminate();
    }
};


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sockaddr_in serverAddr;
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, 50) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Server listening on port " << PORT << "...\n";

    ThreadPool pool(22);
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed\n";
            continue;
        }
        char client_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr, INET_ADDRSTRLEN);
        cout << "Connection accepted from " << client_addr << ":" << ntohs(clientAddr.sin_port) << endl;
        pool.addTask(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
