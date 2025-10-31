#include <iostream>
#include <cmath>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 54000
#define BUFFER_SIZE 1024

using namespace std;

void send_message(int client_fd, const string &msg) {
    string message = msg + "\n\n";  // delimiter to mark message end
    send(client_fd, message.c_str(), message.size(), 0);
}

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
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        return 1;
    }

    cout << "Calculator server listening on port " << PORT << endl;

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return 1;
    }

    cout << "Client connected" << endl;

    while (true) {
        string menu =
            "--- Calculator Menu ---\n"
            "1. Addition (+)\n"
            "2. Subtraction (-)\n"
            "3. Multiplication (*)\n"
            "4. Division (/)\n"
            "5. Sine (sin)\n"
            "6. Cosine (cos)\n"
            "7. Tangent (tan)\n"
            "8. Exit\n"
            "Enter your choice: ";

        send_message(client_fd, menu);

        string choice_str;
        if (!recv_message(client_fd, choice_str)) break;

        int choice = 0;
        try {
            choice = stoi(choice_str);
        } catch (...) {
            send_message(client_fd, "Invalid input. Please enter a number.");
            continue;
        }

        if (choice == 8) {
            send_message(client_fd, "Goodbye!");
            break;
        }

        double num1 = 0, num2 = 0, result = 0;

        if (choice >= 1 && choice <= 4) {
            send_message(client_fd, "Enter first number:");
            string n1;
            if (!recv_message(client_fd, n1)) break;

            send_message(client_fd, "Enter second number:");
            string n2;
            if (!recv_message(client_fd, n2)) break;

            try {
                num1 = stod(n1);
                num2 = stod(n2);
            } catch (...) {
                send_message(client_fd, "Invalid number input.");
                continue;
            }

            if (choice == 4 && num2 == 0) {
                send_message(client_fd, "Error: Division by zero.");
                continue;
            }

            switch (choice) {
                case 1: result = num1 + num2; break;
                case 2: result = num1 - num2; break;
                case 3: result = num1 * num2; break;
                case 4: result = num1 / num2; break;
            }
        } else if (choice >= 5 && choice <= 7) {
            send_message(client_fd, "Enter angle in degrees:");
            string angle_str;
            if (!recv_message(client_fd, angle_str)) break;

            double angle_deg;
            try {
                angle_deg = stod(angle_str);
            } catch (...) {
                send_message(client_fd, "Invalid angle input.");
                continue;
            }

            double angle_rad = angle_deg * M_PI / 180.0;
            switch (choice) {
                case 5: result = sin(angle_rad); break;
                case 6: result = cos(angle_rad); break;
                case 7: result = tan(angle_rad); break;
            }
        } else {
            send_message(client_fd, "Invalid choice, try again.");
            continue;
        }

        send_message(client_fd, "Result: " + to_string(result));
        // Then loop again and show menu
    }

    close(client_fd);
    close(server_fd);
    cout << "Server closed connection\n";

    return 0;
}
