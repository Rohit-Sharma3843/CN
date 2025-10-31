#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
using namespace std;
const string GENERATOR = "1101";
char xorbit(char a, char b) {
    return (a == b) ? '0' : '1';
}
string mod2div(const string &dividend, const string &divisor) {
    int pick = divisor.size();
    string tmp = dividend.substr(0, pick);

    int n = dividend.size();
    while (pick < n) {
        if (tmp[0] == '1') {
            for (size_t i = 0; i < divisor.size(); ++i)
                tmp[i] = xorbit(tmp[i], divisor[i]);
        } else {
            for (size_t i = 0; i < divisor.size(); ++i)
                tmp[i] = xorbit(tmp[i], '0');
        }
        tmp.erase(tmp.begin());
        tmp.push_back(dividend[pick]);
        ++pick;
    }

    if (tmp[0] == '1') {
        for (size_t i = 0; i < divisor.size(); ++i)
            tmp[i] = xorbit(tmp[i], divisor[i]);
    } else {
        for (size_t i = 0; i < divisor.size(); ++i)
            tmp[i] = xorbit(tmp[i], '0');
    }
    return tmp.substr(1);
}
int main() {
    cout << "===CRC Receiver===\n";
    cout<<"Receiver IP is : 127.0.0.47"<<endl;
    cout << "Generator Polynomial is : " << GENERATOR << "\n";
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("socket");
        return 1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(54000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.47"); 
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(listen_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_sock);
        return 1;
    }
    if (listen(listen_sock, 1) < 0) {
        perror("listen");
        close(listen_sock);
        return 1;
    }
    cout << "Receiver listening on 127.0.0.47:54000...\n";
    sockaddr_in client{};
    socklen_t clientlen = sizeof(client);
    int client_sock = accept(listen_sock, (sockaddr*)&client, &clientlen);
    if (client_sock < 0) {
        perror("accept");
        close(listen_sock);
        return 1;
    }
    cout << "Sender connected.\n";
    uint32_t netlen;
    recv(client_sock, &netlen, sizeof(netlen), MSG_WAITALL);
    uint32_t len = ntohl(netlen);
    string buffer(len, '0');
    recv(client_sock, &buffer[0], len, MSG_WAITALL);
    cout << "Received Frame is : " << buffer << "\n";
    string remainder = mod2div(buffer, GENERATOR);
    cout << "Calculated CRC is : " << remainder << "\n";
    bool valid = true;
    for (char c : remainder)
        if (c != '0') valid = false;
    if (valid)
        cout << "CRC Check: No Error Detected. So frame is accepted.\n";
    else
        cout << "CRC Check: Error Detected. So frame is rejected.\n";
    close(client_sock);
    close(listen_sock);
    return 0;
}
