#include <iostream>
#include <vector>
#include <stdio.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include "cpp-httplib/httplib.h"

#define RANGE_COUNT 5
#define DOWN_DIR    "download"
using namespace httplib;
class P2PClient
{
    public:
        P2PClient(){
        }
        ~P2PClient(){
        }
        bool Start() {
            while(1) {
                UserSelect();
            }
            return true;
        }
    private:
        bool UserSelect() {
            int s;
            std::vector<std::string> list;
            while(1) {
                std::cout << "1. Find nearby host\n";
                std::cout << "2. Print nearby host\n";
                std::cout << "3. Get user's file list\n";
                std::cout << "4. Download file\n";
                std::cout << "Please input your mind:";
                fflush(stdout);
                std::cin >> s;
                switch(s){
                    case 1:
                        list.clear();
                        GetHostList(list);
                        FindNearByHost(list);
                        break;
                    case 2:
                        PrintNearByHost();
                        break;
                    case 3:
                        GetFileList();
                        break;
                    case 4:
                        GetFileRange();
                        DownLoadFile();
                        break;
                    default:
                        std::cout << "selection error\n";
                        break;
                }
            }
        }
        bool GetHostList(std::vector<std::string> &list) {
            struct ifaddrs *addrs = NULL;
            uint32_t net_seg, end_host;
            getifaddrs(&addrs);
            while(addrs) {
                if (addrs->ifa_addr->sa_family == AF_INET && strncasecmp(addrs->ifa_name, "lo", 2)) {
                    sockaddr_in *addr = (sockaddr_in*)addrs->ifa_addr;
                    sockaddr_in *mask = (sockaddr_in*)addrs->ifa_netmask;
                    net_seg = ntohl(addr->sin_addr.s_addr & mask->sin_addr.s_addr);
                    end_host = ntohl(~mask->sin_addr.s_addr);
                    for (int i = 1; i < end_host; i++) {
                        struct in_addr addr;
                        addr.s_addr = htonl(net_seg + i); 
                        std::string str = inet_ntoa(addr);
                        list.push_back(str);
                    }   
                }   
                addrs = addrs->ifa_next;
            }
            return true;
        }
        bool FindNearByHost(std::vector<std::string> &list) {
            _host_list.clear();
            for (int i = 0; i < list.size(); i++) {
                std::string ip = list[i];
                httplib::Client cli(ip.c_str(), 9000);
                auto res = cli.Get("/hostpair");
                if (res && res->status == 200) {
                    _host_list.push_back(ip);
                    std::cout << "host "<< ip << " pair success\n";
                    continue;
                }
                std::cout << "host "<< ip << " pair failed\n";
            }
            //_host_list.push_back("192.168.122.132");
            return true;
        }
        bool PrintNearByHost(){
            std::cout << "A total of "<<_host_list.size()<<" users were found nearby:\n";
            for (int i = 0; i < _host_list.size(); i++) {
                std::cout<< i << ". " << _host_list[i] << std::endl;
            }   
            std::cout << "Please enter the host address you want to view (enter -1 to exit):";
            fflush(stdout);
            std::cin >> _cur_view_host;
            if (_cur_view_host < 0 || _cur_view_host > _host_list.size()) {
                std::cout << "Host address is not found\n";
                _cur_view_host = -1;
                return false;
            }
            return true;
        }
        bool GetFileList() {
            if (_cur_view_host == -1){
                std::cout << "Please enter the host address you want to view\n";
                return false;
            }
            httplib::Client cli(_host_list[_cur_view_host].c_str(), 9000);
            auto res = cli.Get("/filelist");
            if (res && res->status != 200) {
                return false;
            }
            std::string sep = "\r\n";

            _file_list.clear();
            size_t idx = 0, pos;
            while(1) {
                pos = res->body.find(sep, idx);
                if (pos == std::string::npos) {
                    break;
                }
                _file_list.push_back(res->body.substr(idx, pos - idx));
                idx = pos + sep.size();
            }
            if (idx < (res->body.size() - 1)) {
                _file_list.push_back(res->body.substr(idx));
            }

            std::cout << "A total of "<<_file_list.size()<<" files were shared:\n";
            for (int i = 0; i < _file_list.size(); i++) {
                std::cout << i << ". " <<_file_list[i] << std::endl;
            }
            std::cout << "Please enter the file number you want to download(enter -1 to exit):";
            fflush(stdout);
            std::cin >> _cur_down_file;
            if (_cur_down_file < 0 || _cur_down_file > _file_list.size()){
                std::cout << "File name is not found\n";
                _cur_down_file = -1;
                return false;
            }

            return true;
        }
        bool GetFileRange() {
            httplib::Client cli(_host_list[_cur_view_host].c_str(), 9000);
            auto res = cli.Head(_file_list[_cur_down_file].c_str());
            if (res && res->status != 200) {
                return false;
            }
            if (res->has_header("Content-Length")) {
                std::stringstream ss;
                ss << res->get_header_value("Content-Length", 0);
                ss >> _cur_down_size;
            }else {
                std::cout<<"Unable to retrieve the text to determine the length, it will be downloaded separately\n";
                _cur_down_size = -1;
            }
            return true;
        }
        bool DownLoadFile(){
            int64_t range_start, range_end, range_size;
            int pid[RANGE_COUNT];
            range_size = _cur_down_size / RANGE_COUNT;
            for (int i = 0; i < RANGE_COUNT; i++) {
                range_start = i * range_size;
                if (i == (RANGE_COUNT - 1)) {
                    range_end = _cur_down_size - 1;
                }else {
                    range_end = (i + 1) * range_size - 1;
                }
                pid[i] = fork();
                if (pid[i] == 0) {
                    httplib::Client range_cli(_host_list[_cur_view_host].c_str(), 9000);
                    httplib::Headers hdr = { httplib::make_range_header(range_start, range_end) };
                    auto res = range_cli.Get(_file_list[_cur_down_file].c_str(), hdr);
                    if (res && res->status == 206) {
                        WriteFile(_file_list[_cur_down_file], res->body, range_start, range_end);
                    }else {
                        std::cout<<"download failed:"<<res->body<<std::endl;
                    }
                    exit(0);
                }
            }
            for (int i = 0 ; i < RANGE_COUNT; i++) {
                waitpid(pid[i], NULL, 0);
            }
            std::cout<<"download file success\n";
            return true;
        }
        bool WriteFile(std::string &filename, std::string &body, int64_t start, int64_t end) {
            int64_t write_len = end - start + 1;
            if (access(DOWN_DIR, F_OK) < 0) {
                mkdir(DOWN_DIR, 0775);
            }
            size_t pos = filename.find_last_of("/");
            std::string path = DOWN_DIR + filename.substr(pos);

            int fd = open(path.c_str(), O_CREAT|O_WRONLY, 0664);
            if (fd < 0) {
                perror("open error");
                return false;
            }
            lseek(fd, start, SEEK_SET);
            if (write(fd, &body[0], write_len) < 0) {
                perror("write error");
                return false;
            }
            close(fd);
            
            return true;
        }
        std::string dump_headers(const Headers &headers) {
            std::string s;
            char buf[BUFSIZ];
            for (const auto &x : headers) {
                snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
                s += buf;
            }
            return s;
        }
    private:
        std::vector<std::string> _host_list;
        std::vector<std::string> _file_list;
        int _cur_view_host;
        int _cur_down_file;
        int64_t _cur_down_size;
};
