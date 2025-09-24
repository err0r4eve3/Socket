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
            std::cout << "[服务器] " << std::string(buf, n) << std::endl;
        }
        else if (n == 0) {
            std::cout << "服务器关闭连接\n";
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

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&addr, sizeof(addr));
    std::cout << "已连接服务器\n";

    std::thread t1(recvThread, sock);
    std::thread t2(sendThread, sock);

    t1.join();
    t2.join();

    closesocket(sock);
    WSACleanup();
}
