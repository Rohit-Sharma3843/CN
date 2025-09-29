// receiver.cpp
// Compile: g++ receiver.cpp -o receiver
// Run: ./receiver
// Then run sender

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

char xorbit(char a, char b)
{
    return (a == b) ? '0' : '1';
}

string mod2div(const string &dividend, const string &divisor)
{
    int pick = divisor.size();
    string tmp = dividend.substr(0, pick);

    int n = dividend.size();
    while (pick < n)
    {
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
        tmp.erase(tmp.begin());
        tmp.push_back(dividend[pick]);
        ++pick;
    }

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
    return tmp.substr(1);
}

int main()
{
    // Use the same generator as sender
    string generator;
    cout << "Enter generator polynomial in binary (same as sender, e.g. 1101): ";
    cin >> generator;
    if (generator.size() < 2)
    {
        cerr << "Generator too short.\n";
        return 1;
    }

    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(54000);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listen_sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(listen_sock);
        return 1;
    }

    if (listen(listen_sock, 1) < 0)
    {
        perror("listen");
        close(listen_sock);
        return 1;
    }

    cout << "Receiver listening on port 54000. Waiting for sender...\n";
    sockaddr_in client;
    socklen_t clientlen = sizeof(client);
    int client_sock = accept(listen_sock, (sockaddr *)&client, &clientlen);
    if (client_sock < 0)
    {
        perror("accept");
        close(listen_sock);
        return 1;
    }
    cout << "Sender connected.\n";

    // receive length first
    uint32_t netlen;
    ssize_t r = recv(client_sock, &netlen, sizeof(netlen), MSG_WAITALL);
    if (r != sizeof(netlen))
    {
        perror("recv len");
        close(client_sock);
        close(listen_sock);
        return 1;
    }
    uint32_t len = ntohl(netlen);

    string buffer;
    buffer.resize(len);
    ssize_t received = 0;
    while (received < (ssize_t)len)
    {
        ssize_t rd = recv(client_sock, &buffer[received], len - received, 0);
        if (rd <= 0)
        {
            perror("recv data");
            break;
        }
        received += rd;
    }

    cout << "Received frame: " << buffer << " (len=" << received << ")\n";

    // Validate using CRC
    // For validation, perform mod2 division on the full received frame (data+crc)
    string remainder = mod2div(buffer, generator);
    cout << "Remainder after dividing by generator: " << remainder << "\n";

    bool valid = true;
    for (char c : remainder)
        if (c != '0')
        {
            valid = false;
            break;
        }

    if (valid)
        cout << "CRC Check: NO ERROR detected (remainder all zeros).\n";
    else
        cout << "CRC Check: ERROR detected (non-zero remainder).\n";

    close(client_sock);
    close(listen_sock);
    cout << "Receiver exiting.\n";
    return 0;
}
