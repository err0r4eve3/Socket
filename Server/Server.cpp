#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#pragma comment(lib, "ws2_32.lib")

std::atomic<bool> running(true);

void recvThread(SOCKET sock) {
    char buf[1024];
    while (running) {
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0) {
            std::cout << "[客户端] " << std::string(buf, n) << std::endl;
        }
        else if (n == 0) {
            std::cout << "客户端关闭连接\n";
            running = false;
            break;
        }
        else {
            std::cout << "recv 错误: " << WSAGetLastError() << "\n";
            running = false;
            break;
        }
    }
}

void sendThread(SOCKET sock) {
    std::string line;
    while (running && std::getline(std::cin, line)) {
        if (line == "/quit") {
            running = false;
            shutdown(sock, SD_SEND);
            break;
        }
        send(sock, line.c_str(), (int)line.size(), 0);
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSock, (sockaddr*)&addr, sizeof(addr));
    listen(serverSock, 1);

    std::cout << "等待客户端连接...\n";
    SOCKET clientSock = accept(serverSock, NULL, NULL);
    std::cout << "客户端已连接！\n";

    std::thread t1(recvThread, clientSock);
    std::thread t2(sendThread, clientSock);

    t1.join();
    t2.join();

    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();
}
