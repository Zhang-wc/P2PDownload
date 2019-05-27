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
    private:
        std::string _version;
        std::string _statu;
        std::string _desc;
        list_map _headers;
        std::string _hdr_str;
        Socket::TcpSocket _sock;
};

#endif
