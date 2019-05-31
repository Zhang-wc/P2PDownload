#ifndef __M_HTTP_H__
#define __M_HTTP_H__ 
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "tcpsocket.hpp"

typedef std::unordered_map<std::string, std::string> list_map;

class Request
{
    public:
        Request(){
        }
        ~Request(){
        }
        std::string & GetMethod() {
            return _method;
        }
        std::string & GetPath() {
            return _path;
        }
        std::string & GetQuery() {
            return _query;
        }
        std::string & GetVersion() {
            return _version;
        }
        void SetMethod(const std::string &method) {
            _method = method;
        }
        void SetPath(const std::string &path) {
            _path = path;
        }
        void SetVersion(const std::string &version) {
            _version = version;
        }
        void SetRequestLine(const std::string &method, const std::string &path, const std::string &version){
            SetMethod(method);
            SetPath(path);
            SetVersion(version);
        }
        void SetHeader(const std::string &key, const std::string &val) {
            _headers[key] = val;
        }
        void SetSock(Socket::TcpSocket sock) {
            _sock.SetFd(sock.GetFd());
        }
        std::string & MakeRequestHeader() {
            std::stringstream ss;
            ss << _method << " " << _path << " " << _version << "\r\n";
            for (auto it = _headers.begin(); it != _headers.end(); it++){
                ss << it->first << ": " << it->second << "\r\n";
            }
            ss << "\r\n";
            _hdr_str = ss.str();
            return _hdr_str;
        }
        bool ParseRequestHeader(){
            return true;
        }
        bool RecvRequestHeader() {
            return true;
        }
        bool SendRequestHeader() {
            MakeRequestHeader();
            CHECK_RET(_sock.Send(_hdr_str.c_str(), _hdr_str.size()));
            return true;
        }
    private:
        std::string _method;
        std::string _url;
        std::string _path;
        std::string _query;
        std::string _version;
        list_map _headers;
        std::string _hdr_str;
        Socket::TcpSocket _sock;
};

class Response
{
    public:
        Response(){
        }
        ~Response(){
        }
        void SetVersion(std::string &version) {
            _version = version;
        }
        void SetStatus(std::string &statu) {
            _statu = statu;
        }
        void SetDesc(std::string &desc) {
            _desc = desc;
        }
        void SetHeader(const std::string &key, const std::string &val) {
            _headers[key] = val;
        }
        void SetSock(Socket::TcpSocket sock) {
            _sock.SetFd(sock.GetFd());
        }
        std::string & GetVersion() {
            return _version;
        }
        std::string & GetStatus() {
            return _statu;
        }
        std::string & GetDesc() {
            return _desc;
        }
        bool ParseResponseHeader() {
            return true;
        }
        std::string & MakeResponseHeader() {
            return _hdr_str;
        }
        bool RecvResponseHeader() {
            return true;
        }
        bool SendResponseHeader() {
            return true;
        }
        int64_t GetContentLength() {
            auto it = _headers.find("Content-Length");
            if (it == _headers.end()) {
                return -1;
            }
            int64_t length;
            std::stringstream ss;
            ss << it->second;
            ss >> length;
            return length;
        }
        bool GetContentBody(std::string &buf) {
            char tmp[MAX_BUFFER];
            if (_cont_len != -1) {
                if (_recv_len >= _cont_len) return false;
                int rlen = (_cont_len - _recv_len) > MAX_BUFFER ? MAX_BUFFER : (_cont_len - _recv_len);
                int ret = _sock.Recv(tmp, rlen);
                if (ret <= 0) {
                    return false;
                }
                buf.assign(tmp, ret);
            }
            return true;
        }
        bool GetContentLine(std::string &buf) {
            char tmp[MAX_BUFFER];
            buf.clear();
            while(1) {
                int ret = _sock.RecvPeek(tmp, MAX_BUFFER);
                if (ret <= 0) {
                    return false;
                }
                std::string str;
                str.assign(tmp, ret);
                size_t pos = str.find_first_of("\n");
                if (pos == std::string::npos) {
                    buf += str;
                    _sock.Recv(tmp, ret);
                }else {
                    buf += str.substr(0, pos);
                    _sock.Recv(tmp, pos);
                    break;
                }
            }
            return true;
        }
    private:
        int64_t _cont_len;
        int64_t _recv_len;
        std::string _version;
        std::string _statu;
        std::string _desc;
        list_map _headers;
        std::string _hdr_str;
        Socket::TcpSocket _sock;
};

#endif
