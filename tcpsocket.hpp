#ifndef __M_TC_H__
#define __M_TC_H__
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define CHECK_RET(q) if((q) == false){return false;}
#define MAX_BUFFER  4096

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

class TcpSocket
{
    private:
        int _sock;
    public:
        TcpSocket() : _sock(-1) {}
        void SetFd(int sock) {
            _sock = sock;
        }
        int GetFd() {
            return _sock;
        }
        bool Socket() {
            _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_sock < 0) {
                perror("socket error");
                return false;
            }
            int opt = 1;
            setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
            return true;
        }
        bool Bind(uint16_t port, std::string ip = "0.0.0.0") {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            if (bind(_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("bind error");
                return false;
            }
            return true;
        }
        bool Connect(uint16_t port, std::string ip) {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            if (connect(_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("connect error");
                return false;
            }
            return true;
        }
        bool Listen(int baklog = 5) {
            if (listen(_sock, baklog) < 0) {
                perror("listen error");
                return false;
            }
            return true;
        }
        int Accept(sockaddr_in *addr = NULL) {
            int newsock;
            sockaddr_in _addr;
            socklen_t len = sizeof(sockaddr_in);
            newsock = accept(_sock, (sockaddr*)&_addr, &len);
            if (newsock < 0) {
                perror("accept error");
                return -1;
            }
            if (addr) {
                memcpy(addr, &_addr, len);
            }
            return newsock;
        }
        void Close() {
            close(_sock);
            _sock = -1;
        }
        bool ServerInit(uint16_t port, std::string ip = "0.0.0.0") {
            CHECK_RET(Socket());
            CHECK_RET(Bind(port, ip));
            CHECK_RET(Listen());
            return true;
        }
        bool ClientInit(uint16_t port, std::string ip = "0.0.0.0") {
            CHECK_RET(Socket());
            CHECK_RET(Connect(port, ip));
            return true;
        }
        int Recv(char *buf, int len) {
            ssize_t ret = recv(_sock, buf, len, 0);
            if (ret <= 0) {
                perror("recv error");
                return ret;
            }
            return ret;
        }
        int RecvPeek(char *buf, int len) {
            ssize_t ret = recv(_sock, buf, len, MSG_PEEK);
            if (ret <= 0) {
                perror("recv error");
                return ret;
            }
            return ret;
        }

        bool Send(const char *buf, int len){
            int slen = 0;
            while(slen < len) {
                ssize_t ret = send(_sock, buf + slen, len - slen, 0);
                if (ret < 0) {
                    perror("send error");
                    return false;
                }
                slen += ret;
            }
            return true;
        }
        bool SendChunk(const char *buf, int len) {
            if (len == 0) {
                CHECK_RET(Send("0\r\n\r\n", 5));
            }else {
                char tmp[256] = {0};
                int ret = sprintf(tmp, "%x\r\n", len);
                CHECK_RET(Send(tmp, ret));
                CHECK_RET(Send(buf, len));
                CHECK_RET(Send("\r\n", 2));
            }
            return true;
        }
};

#endif
