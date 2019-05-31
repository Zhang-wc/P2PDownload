#include <sys/types.h>
#include <sys/stat.h>
#include "tcpsocket.hpp"
#include "threadpool.hpp"
#include "http.hpp"

#define DOWNLOAD_DIR    "./Download"
#define HOSTNAME_LEN    256
#define SERVER_PORT     9000

class Client
{
    public:
        Client(){
        }
        ~Client(){
        }
        bool StartClient() {
            FindNearByHost();
        }
    private:
        //tcp：通过本机IP地址和子网掩码得到网段,通过遍历所有主机号进行广播获取到局域网内的其它共享用户
        //udp广播实现起来更加简单一点~~
        bool FindNearByHost() {
            std::vector<std::string> host_list;
            Socket::GetBroadcastAddr(host_list);

            Response rsp;
            Request req;
            req.SetRequestLine("GET", "/hostpair", "HTTP/1.1");
            req.SetHeader("Content-Length", "0");
            req.SetHeader("Connection", "close");
            //所有主机号中全0和全1不能使用
            for (int i = 0; i < host_list.size(); i++){
                Socket::TcpSocket sock;
                if (sock.ClientInit(SERVER_PORT, host_list[i]) == false) continue;
                req.SetSock(sock);
                rsp.SetSock(sock);
                if (req.SendRequestHeader() == false) continue;
                if (rsp.RecvResponseHeader() == false) continue;
                if (rsp.ParseResponseHeader() == false) continue;
                if (rsp.GetStatus() == "200") {
                    _host_list.push_back(host_list[i]);
                }
            }
            return true;
        }
        bool PrintHostList() {
            std::cout << "A total of "<<_host_list.size()<<"users were found nearby:\n";
            for (int i = 0; i < _host_list.size(); i++) {
                std::cout<< "\t" << _host_list[i] << std::endl;
            }
            std::cout << "Please enter the host address you want to view (enter quit to exit):";
            fflush(stdout);
            std::cin >> _cur_view_host;
            if (_cur_view_host == "quit") {
                return false;
            }
            for (int i = 0; i < _host_list.size(); i++) {
                if (_host_list[i] == _cur_view_host) {
                    return true;
                }
            }
            std::cout << "Host address is not found\n";
            return true;
        }
        bool GetFileList(std::string &path) {
            Socket::TcpSocket sock;
            CHECK_RET(sock.ClientInit(SERVER_PORT, _cur_view_host));

            Response rsp;
            Request req;
            req.SetSock(sock);
            rsp.SetSock(sock);

            req.SetRequestLine("GET", path, "HTTP/1.1");
            req.SetHeader("Content-Length", "0");
            req.SetHeader("Connection", "close");
            CHECK_RET(req.SendRequestHeader());
            CHECK_RET(rsp.RecvResponseHeader());
            CHECK_RET(rsp.ParseResponseHeader());
            if (rsp.GetStatus() != "200") {
                std::cout << "Error: "<< rsp.GetDesc()<<std::endl;
                return false;
            }

            std::string buf;
            while(rsp.GetContentLine(buf)) {
                _file_list.push_back(buf);
            }
            return true;
        }
        bool PrintFileList() {
            std::cout <<"Host: "<<_cur_view_host<<" File List: \n";
            for (int i = 0; i < _file_list.size(); i++) {
                std::cout << "\t" << _file_list[i] << std::endl;
            }
            std::cout <<"Please enter the file you want to download (enter quit to exit):";
            fflush(stdout);
            std::cin >> _cur_down_file;
            if (_cur_down_file == "quit") {
                return false;
            }
            for (int i = 0; i < _file_list.size(); i++) {
                if (_file_list[i] == _cur_down_file) {
                    return true;
                }
            }
            std::cout << "File name is not found\n";
            return false;
        }
        bool GetFileLength() {
            Socket::TcpSocket sock;
            CHECK_RET(sock.ClientInit(SERVER_PORT, _cur_view_host));

            Response rsp;
            Request req;
            req.SetSock(sock);
            rsp.SetSock(sock);

            req.SetRequestLine("HEAD", _cur_down_file, "HTTP/1.1");
            req.SetHeader("Content-Length", "0");
            req.SetHeader("Connection", "close");
            CHECK_RET(req.SendRequestHeader());
            CHECK_RET(rsp.RecvResponseHeader());
            CHECK_RET(rsp.ParseResponseHeader());
            if (rsp.GetStatus() != "200") {
                std::cout << "Error: "<< rsp.GetDesc()<<std::endl;
                return false;
            }
            _cur_file_size = rsp.GetContentLength();
            return true;
        }
        bool DistributeDownLoad() {
            if (_cur_file_size < 0) {
                std::cout << "File: "<<_cur_down_file << " is not support range download\n";
                std::cout << "Will be start download:\n";
                _range_count = 1;
            }else {
                if (_cur_file_size < 1024 * 1024) {
                    std::cout << "Scope is too small, use single download:\n";
                    _range_count = 1;
                }else {
                    _range_count = 5;
                }
            }
            return true;
        }
        bool FileDownLoad() {
            int64_t range_size = _cur_file_size / _range_count;
            for (int i = 0; i < _range_count; i++){
                int64_t start = i * range_size;
                int64_t end = (i + 1) * range_size - 1;
                if (i == (_range_count - 1)) {
                    end += (range_size - 1) - end;
                }
                int pid = fork();
                if (pid == 0) {
                    exit(0);
                }
            }
            return true;
        }
        void SetSharedPath(const std::string &path = "./download") {
            _shared_path = path;
            if (access(_shared_path.c_str(), F_OK) < 0) {
                mkdir(_shared_path.c_str(), 0775);
            }
        }
    private:
        std::string _shared_path;   //下载文件的存储目录
        std::string _cur_view_host; //当前要查看的用户主机地址
        std::string _cur_down_file; //当前要下载的文件
        int64_t _cur_file_size;     //当前下载的文件总长度
        int _range_count;
        std::vector<std::string> _host_list;//匹配到的附近用户地址
        std::vector<std::string> _file_list;//当前用户主机上共享的文件url
        ThreadPool *_tp;
};

int main() 
{
    Client client;
    client.StartClient();
    return 0;
}
