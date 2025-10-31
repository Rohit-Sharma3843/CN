#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 54000
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        return 1;
    }

    // Bind
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        return 1;
    }

    cout << "Server listening on port " << PORT << endl;

    // Accept
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return 1;
    }
    cout << "Client connected" << endl;

    // Receive filename
    char filename[BUFFER_SIZE];
    memset(filename, 0, BUFFER_SIZE);
    int bytes_received = read(client_fd, filename, BUFFER_SIZE - 1);
    if (bytes_received <= 0) {
        cerr << "Failed to receive filename" << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }
    filename[bytes_received] = '\0';

    cout << "Requested filename: " << filename << endl;

    // Build source and destination paths
    string source_path = "./src/" + string(filename);
    string dest_path = "./dest/" + string(filename);

    // Open source file
    ifstream source_file(source_path, ios::binary);
    if (!source_file.is_open()) {
        string err = "Error: File not found or cannot open source file.\n";
        send(client_fd, err.c_str(), err.size(), 0);
        cerr << err;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Open destination file
    ofstream dest_file(dest_path, ios::binary);
    if (!dest_file.is_open()) {
        string err = "Error: Cannot open destination file for writing.\n";
        send(client_fd, err.c_str(), err.size(), 0);
        cerr << err;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Copy content
    dest_file << source_file.rdbuf();

    source_file.close();
    dest_file.close();

    string success = "File copied successfully.";
    send(client_fd, success.c_str(), success.size(), 0);
    cout << success << endl;

    close(client_fd);
    close(server_fd);

    return 0;
}
