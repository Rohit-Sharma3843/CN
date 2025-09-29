// sender.cpp
// Compile: g++ sender.cpp -o sender
// Run: ./sender
// Make sure receiver is running on localhost:54000

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <random>
#include <string>

using namespace std;

// Perform XOR on two chars '0'/'1'
char xorbit(char a, char b)
{
    return (a == b) ? '0' : '1';
}

// Modulo-2 division: returns remainder string of length (generator.size()-1)
string mod2div(const string &dividend, const string &divisor)
{
    int pick = divisor.size();
    string tmp = dividend.substr(0, pick);

    int n = dividend.size();
    while (pick < n)
    {
        if (tmp[0] == '1')
        {
            // XOR with divisor
            for (size_t i = 0; i < divisor.size(); ++i)
                tmp[i] = xorbit(tmp[i], divisor[i]);
        }
        else
        {
            // XOR with 0...0 (no change except shift)
            for (size_t i = 0; i < divisor.size(); ++i)
                tmp[i] = xorbit(tmp[i], '0');
        }
        // shift left and bring next bit
        tmp.erase(tmp.begin());
        tmp.push_back(dividend[pick]);
        ++pick;
    }

    // last step
    if (tmp[0] == '1')
    {
        for (size_t i = 0; i < divisor.size(); ++i)
            tmp[i] = xorbit(tmp[i], divisor[i]);
    }
    else
    {
        for (size_t i = 0; i < divisor.size(); ++i)
            tmp[i] = xorbit(tmp[i], '0');
    }

    // remainder is last (divisor.size()-1) bits
    return tmp.substr(1);
}

int main()
{
    // Choose generator polynomial as binary string.
    // Example "1101" corresponds to x^3 + x^2 + 1 (degree r = 3)
    string generator;
    cout << "Enter generator polynomial in binary (e.g. 1101): ";
    cin >> generator;
    if (generator.size() < 2)
    {
        cerr << "Generator too short.\n";
        return 1;
    }

    string data;
    cout << "Enter data bits (binary string, e.g. 1011001): ";
    cin >> data;

    // Append r zeros
    int r = generator.size() - 1;
    string appended = data;
    appended.append(r, '0');

    // Calculate remainder
    string remainder = mod2div(appended, generator);

    // Frame to send: original data + remainder
    string frame = data + remainder;

    cout << "Computed CRC bits: " << remainder << "\n";
    cout << "Frame to transmit: " << frame << "\n";

    // Ask to introduce error
    char choice;
    cout << "Introduce error? (y/n): ";
    cin >> choice;
    if (choice == 'y' || choice == 'Y')
    {
        int pos;
        cout << "Enter bit position to flip (0-based, 0 is leftmost): ";
        cin >> pos;
        if (pos < 0 || pos >= (int)frame.size())
        {
            cout << "Position out of bounds. Choosing random position instead.\n";
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, frame.size() - 1);
            pos = dis(gen);
            cout << "Random pos = " << pos << "\n";
        }
        // Flip bit
        frame[pos] = (frame[pos] == '0') ? '1' : '0';
        cout << "Frame after introducing error: " << frame << "\n";
    }

    // Basic TCP client to send frame to receiver on localhost:54000
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    cout << "Connecting to receiver...\n";
    if (connect(sock, (sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        close(sock);
        return 1;
    }
    cout << "Connected. Sending frame...\n";

    // send frame length first (as 4 bytes network order) so receiver knows how many bytes to expect
    uint32_t len = htonl((uint32_t)frame.size());
    if (send(sock, &len, sizeof(len), 0) != sizeof(len))
    {
        perror("send len");
    }
    // send frame bytes
    ssize_t sent = send(sock, frame.c_str(), frame.size(), 0);
    if (sent < 0)
        perror("send frame");
    else
        cout << "Sent " << sent << " bytes.\n";

    close(sock);
    cout << "Sender done.\n";
    return 0;
}
