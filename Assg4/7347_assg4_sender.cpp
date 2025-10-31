
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
using namespace std;
using steady_clock = chrono::steady_clock;

const int TOTAL_FRAMES = 10;
const int WINDOW_SIZE  = 5;
const int SEQ_RANGE    = 5;
const double TIMEOUT   = 3.0;
double now_seconds() {
    return chrono::duration<double>(steady_clock::now().time_since_epoch()).count();
}
bool recv_available(int sock, string &recvBuf) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    timeval tv{0, 0}; // immediate return
    int rv = select(sock + 1, &fds, NULL, NULL, &tv);
    if (rv > 0 && FD_ISSET(sock, &fds)) {
        char tmp[4096];
        int n = recv(sock, tmp, sizeof(tmp), 0);
        if (n > 0) {
            recvBuf.append(tmp, n);
            return true;
        }
    }
    return false;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(54000);
    serv.sin_addr.s_addr = inet_addr("127.0.0.47");
    if (connect(sockfd, (sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect"); close(sockfd); return 1;
    }
    cout << "Sender connected to Receiver.\n";
    cout << "Choose protocol:\n1. Go-Back-N\n2. Selective Repeat\n";
    int choice; cin >> choice;
    bool mode_gbn = (choice == 1);
    string modeMsg = string("MODE:") + (mode_gbn ? "G\n" : "S\n");
    send(sockfd, modeMsg.c_str(), modeMsg.size(), 0);
    vector<bool> acked(TOTAL_FRAMES, false);
    vector<double> lastSend(TOTAL_FRAMES, 0.0);
    vector<bool> sentOnce(TOTAL_FRAMES, false);
    int base = 0;     
    int nextToSend = 0; 
    string recvBuf;
    while (base < TOTAL_FRAMES) {
        while (nextToSend < TOTAL_FRAMES && nextToSend < base + WINDOW_SIZE) {
            string frame = "F" + to_string(nextToSend) + "\n";
            send(sockfd, frame.c_str(), frame.size(), 0);
            lastSend[nextToSend] = now_seconds();
            sentOnce[nextToSend] = true;
            int seqNo = nextToSend % SEQ_RANGE;
            cout << "Sender: Frame " << nextToSend << " (seq=" << seqNo << ") | Sent\n";
            nextToSend++;
        }
        bool advanced = false;
        double loop_start = now_seconds();
        while (true) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(sockfd, &fds);
            double min_time_left = TIMEOUT;
            for (int i = base; i < min(nextToSend, TOTAL_FRAMES); ++i) {
                if (!acked[i] && sentOnce[i]) {
                    double left = TIMEOUT - (now_seconds() - lastSend[i]);
                    if (left < min_time_left) min_time_left = left;
                }
            }
            if (min_time_left < 0) min_time_left = 0;
            timeval tv;
            tv.tv_sec = (int)min_time_left;
            tv.tv_usec = (int)((min_time_left - tv.tv_sec) * 1e6);

            int rv = select(sockfd + 1, &fds, NULL, NULL, &tv);

            if (rv < 0) {
                perror("select");
                break;
            }
            if (rv > 0 && FD_ISSET(sockfd, &fds)) {
                char tmp[4096];
                int n = recv(sockfd, tmp, sizeof(tmp), 0);
                if (n <= 0) {
                    break;
                }
                recvBuf.append(tmp, n);
                size_t pos;
                while ((pos = recvBuf.find('\n')) != string::npos) {
                    string line = recvBuf.substr(0, pos);
                    recvBuf.erase(0, pos + 1);
                    if (line.rfind("ACK", 0) == 0) {
                        int ackNo = stoi(line.substr(3));
                        if (mode_gbn) {
                            for (int k = base; k <= ackNo && k < TOTAL_FRAMES; ++k) acked[k] = true;
                            while (base < TOTAL_FRAMES && acked[base]) base++;
                            cout << "Sender: ACK" << ackNo << " | Slide base -> " << base << "\n";
                        } else {
                            if (ackNo >= 0 && ackNo < TOTAL_FRAMES && !acked[ackNo]) {
                                acked[ackNo] = true;
                                cout << "Sender: Frame " << ackNo << " (seq=" << (ackNo % SEQ_RANGE)
                                     << ") | ACK received\n";
                            } else {
                                cout << "Sender: Frame " << ackNo << " | Duplicate ACK (ignored)\n";
                            }
                            while (base < TOTAL_FRAMES && acked[base]) base++;
                        }
                    } else if (line.rfind("NACK", 0) == 0) {
                        int nackNo = stoi(line.substr(4));
                        if (mode_gbn) {
                            cout << "Sender: Received NACK" << nackNo << " | Go-Back-N -> Resend from " << nackNo << "\n";
                            nextToSend = nackNo;
                            break;
                        } else {
                            cout << "Sender: Received NACK" << nackNo << " | SR -> Resend only " << nackNo << "\n";
                            string frame = "F" + to_string(nackNo) + "\n";
                            send(sockfd, frame.c_str(), frame.size(), 0);
                            lastSend[nackNo] = now_seconds();
                            cout << "Sender: Frame " << nackNo << " (seq=" << (nackNo % SEQ_RANGE)
                                 << ") | Retransmission sent\n";
                        }
                    } else {
                        cout << "Sender: Unknown msg from receiver: '" << line << "'\n";
                    }
                }
            }
            bool timeoutOccurred = false;
            for (int i = base; i < min(nextToSend, TOTAL_FRAMES); ++i) {
                if (!acked[i] && sentOnce[i]) {
                    double elapsed = now_seconds() - lastSend[i];
                    if (elapsed >= TIMEOUT) {
                        if (mode_gbn) {
                            cout << "Sender: Frame " << i << " (seq=" << (i % SEQ_RANGE)
                                 << ") | Timeout | Go-Back-N -> Resend from " << i << "\n";
                            nextToSend = i;
                            timeoutOccurred = true;
                            break;
                        } else {
                            cout << "Sender: Frame " << i << " (seq=" << (i % SEQ_RANGE)
                                 << ") | Timeout | SR -> Resend only " << i << "\n";
                            string frame = "F" + to_string(i) + "\n";
                            send(sockfd, frame.c_str(), frame.size(), 0);
                            lastSend[i] = now_seconds();
                        }
                    }
                }
            }
            if (timeoutOccurred) break;
            bool windowAllAcked = true;
            for (int i = base; i < min(nextToSend, TOTAL_FRAMES); ++i) if (!acked[i]) { windowAllAcked = false; break; }
            if (windowAllAcked) { advanced = true; break; }
        }
    }
    close(sockfd);
    cout << "Sender finished.\n";
    return 0;
}
