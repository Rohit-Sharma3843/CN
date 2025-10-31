#include <iostream>
#include <thread>
#include <string>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

#define IP "127.0.0.47"
#define PORT_LISTEN 54000
#define PORT_CONNECT 54001
#define BUFFER_SIZE 1024

void receive_thread_func(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int r = read(sock, buffer, BUFFER_SIZE - 1);
        if (r <= 0) {
            cerr << "Peer1: read error or connection closed\n";
            break;
        }
        buffer[r] = '\0';
        cout << "\nreceiver : " << buffer << "\nYou: " << flush;
    }
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Peer1: socket() failed");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr_listen{};
    addr_listen.sin_family = AF_INET;
    addr_listen.sin_port = htons(PORT_LISTEN);
    if (inet_pton(AF_INET, IP, &addr_listen.sin_addr) <= 0) {
        perror("Peer1: inet_pton listen failed");
        return 1;
    }

    if (bind(listen_fd, (sockaddr*)&addr_listen, sizeof(addr_listen)) < 0) {
        perror("Peer1: bind() failed");
        return 1;
    }

    if (listen(listen_fd, 1) < 0) {
        perror("Peer1: listen() failed");
        return 1;
    }

    cout << "Peer1 listening on " << IP << ":" << PORT_LISTEN << endl;

    // Connect to peer2
    int sock_to_peer;
    while (true) {
        sock_to_peer = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_to_peer < 0) {
            perror("Peer1: socket to peer failed");
            return 1;
        }

        sockaddr_in peer_addr{};
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(PORT_CONNECT);
        if (inet_pton(AF_INET, IP, &peer_addr.sin_addr) <= 0) {
            perror("Peer1: inet_pton peer failed");
            close(sock_to_peer);
            return 1;
        }

        cout << "Peer1: trying to connect to Peer2 at " << IP << ":" << PORT_CONNECT << "\n";
        if (connect(sock_to_peer, (sockaddr*)&peer_addr, sizeof(peer_addr)) == 0) {
            cout << "Peer1: connected to Peer2\n";
            break;
        }
        perror("Peer1: connect failed");
        close(sock_to_peer);
        sleep(1);
    }

    // Accept incoming connection
    sockaddr_in peer_in_addr{};
    socklen_t peer_in_len = sizeof(peer_in_addr);
    int sock_from_peer = accept(listen_fd, (sockaddr*)&peer_in_addr, &peer_in_len);
    if (sock_from_peer < 0) {
        perror("Peer1: accept failed");
        return 1;
    }
    cout << "Peer1: accepted connection from Peer2\n";

    thread recv_thread(receive_thread_func, sock_from_peer);

    string input;
    while (true) {
        cout << "You: ";
        if (!getline(cin, input)) break;
        if (input == "exit") break;
        int s = send(sock_to_peer, input.c_str(), input.size(), 0);
        if (s < 0) {
            perror("Peer1: send failed");
            break;
        }
        cout << "sent : " << input << endl;
    }

    close(sock_to_peer);
    close(sock_from_peer);
    close(listen_fd);
    recv_thread.join();
    return 0;
}
