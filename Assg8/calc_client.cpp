#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 54000
#define BUFFER_SIZE 1024

using namespace std;

bool recv_message(int sock, string &out) {
    out.clear();
    char buffer[BUFFER_SIZE];
    string data;
    while (true) {
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) return false;
        buffer[bytes] = '\0';
        data += buffer;
        if (data.find("\n\n") != string::npos) break;
    }
    out = data.substr(0, data.find("\n\n"));
    return true;
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    while (true) {
        string server_msg;
        if (!recv_message(sock, server_msg)) break;
        cout << server_msg << endl;

        if (server_msg.find("Goodbye!") != string::npos) break;

        string input;
        getline(cin, input);
        input += "\n\n";

        if (send(sock, input.c_str(), input.size(), 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    close(sock);
    return 0;
}
