#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;
const int TOTAL_FRAMES = 10;  
const int SEQ_RANGE    = 5;    
const double ERROR_PROB = 0.20; 
bool isCorrupted(int absNo) {
    return ( (rand() % 100) < int(ERROR_PROB * 100) );
}
int main() {
    srand((unsigned)time(nullptr));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(54000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.47");
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sockfd); return 1;
    }
    if (listen(sockfd, 1) < 0) {
        perror("listen"); close(sockfd); return 1;
    }
    cout << "Receiver listening on 127.0.0.47:54000...\n";
    sockaddr_in cli{};
    socklen_t clen = sizeof(cli);
    int newsock = accept(sockfd, (sockaddr*)&cli, &clen);
    if (newsock < 0) { perror("accept"); close(sockfd); return 1; }
    cout << "Sender connected.\n";
    string inbuf;
    char tmp[4096];
    bool mode_is_gbn = true;
    bool got_mode = false;

    int next_expected = 0; 
    unordered_set<int> sr_buffered;
    int delivered_unique = 0;
    while (delivered_unique < TOTAL_FRAMES) {
        int n = recv(newsock, tmp, sizeof(tmp), 0);
        if (n <= 0) {
            break;
        }
        inbuf.append(tmp, n);
        size_t pos;
        while ((pos = inbuf.find('\n')) != string::npos) {
            string line = inbuf.substr(0, pos);
            inbuf.erase(0, pos + 1);
            if (!got_mode) {
                if (line.size() >= 6 && line.substr(0,5) == "MODE:") {
                    got_mode = true;
                    mode_is_gbn = (line[5] == 'G' || line[5] == 'g');
                    cout << "Receiver: MODE set to " << (mode_is_gbn ? "Go-Back-N" : "Selective Repeat") << "\n";
                } else {
                    cout << "Receiver: Expected MODE but got: '" << line << "'\n";
                }
                continue;
            }
            if (line.size() > 0 && line[0] == 'F') {
                int absNo = stoi(line.substr(1));
                int seqNo = absNo % SEQ_RANGE;
                cout << "Receiver: Got Frame " << absNo << " (seq=" << seqNo << ")\n";
                if (isCorrupted(absNo)) {
                    cout << "Receiver: Frame " << absNo << " (seq=" << seqNo << ") corrupted -> Sending NACK" << absNo << "\n";
                    string nack = "NACK" + to_string(absNo) + "\n";
                    send(newsock, nack.c_str(), nack.size(), 0);
                    continue;
                }
                if (mode_is_gbn) {
                    if (absNo == next_expected) {
                        cout << "Receiver: Valid Frame " << absNo << " (seq=" << seqNo << ") ok -> Sending ACK" << absNo << "\n";
                        string ack = "ACK" + to_string(absNo) + "\n";
                        send(newsock, ack.c_str(), ack.size(), 0);
                        next_expected++;
                        delivered_unique++;
                    } else {
                        int lastAck = next_expected - 1;
                        if (lastAck >= 0) {
                            cout << "Receiver: Out-of-order frame " << absNo << " discarded -> Re-ACK " << lastAck << "\n";
                            string ack = "ACK" + to_string(lastAck) + "\n";
                            send(newsock, ack.c_str(), ack.size(), 0);
                        } else {
                            cout << "Receiver: Out-of-order frame " << absNo << " discarded (no in-order yet)\n";
                        }
                    }
                } else {
                    if (sr_buffered.find(absNo) == sr_buffered.end()) {
                        sr_buffered.insert(absNo);
                        delivered_unique++;
                    } else {
                    }
                    cout << "Receiver: Valid Frame " << absNo << " (seq=" << seqNo << ") ok -> Sending ACK" << absNo << "\n";
                    string ack = "ACK" + to_string(absNo) + "\n";
                    send(newsock, ack.c_str(), ack.size(), 0);
                }
            } else {
                cout << "Receiver: Unknown message '" << line << "'\n";
            }
        }
    }
    close(newsock);
    close(sockfd);
    cout << "Receiver finished.\n";
    return 0;
}
