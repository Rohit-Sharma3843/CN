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
    cout << "===CRC Sender===\n";
    cout << "Generator Polynomial is : " << GENERATOR << "\n";
    string data;
    cout << "Enter data bits to send : ";
    cin >> data;
    int r = GENERATOR.size() - 1;
    string appended = data + string(r, '0');
    string remainder = mod2div(appended, GENERATOR);
    string frame = data + remainder;
    cout << "CRC obtained after division is: " << remainder << "\n";
    cout << "Frame before transmission is : " << frame << "\n";
    char choice;
    cout << "Introduce error? (y/n): ";
    cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        int pos;
        cout << "Enter bit position to flip : ";
        cin >> pos;
        if (pos >= 0 && pos < (int)frame.size()) {
            frame[pos] = (frame[pos] == '0') ? '1' : '0';
            cout << "Frame after error is : " << frame << "\n";
        } else {
            cout << "Invalid position, sending without error.\n";
        }
    } else {
        cout << "Sending frame without error.\n";
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    server.sin_addr.s_addr = inet_addr("127.0.0.47");
    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    uint32_t len = htonl(frame.size());
    send(sock, &len, sizeof(len), 0);
    send(sock, frame.c_str(), frame.size(), 0);
    cout << "Frame sent to receiver.\n";
    close(sock);
    return 0;
}
