#include <netdb.h>
#include <ifaddrs.h>
#include <unordered_map>
#include "tcpsocket.hpp"
#include "threadpool.hpp"

#define DOWNLOAD_DIR    "./Download"
#define HOSTNAME_LEN    256

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
        bool FindNearByHost() {
            struct ifaddrs *addrs = NULL;

            getifaddrs(&addrs);
            while(addrs) {
                if (addrs->ifa_addr->sa_family == AF_INET) {
                    sockaddr_in *addr = (sockaddr_in*)addrs->ifa_addr;
                    sockaddr_in *mask = (sockaddr_in*)addrs->ifa_netmask;
                    std::cout<<"ifaddr name:"<<addrs->ifa_name<<std::endl;
                    std::cout<<"ip:"<<inet_ntoa(addr->sin_addr)<<std::endl;
                    std::cout<<"netmask:"<<inet_ntoa(mask->sin_addr)<<std::endl;
                    std::cout<<"---------------------------------------------------\n";
                }
                addrs = addrs->ifa_next;
            }
            /*获取到的地址是公网地址....
            struct hostent *hent;
            char hostname[HOSTNAME_LEN];
            gethostname(hostname, HOSTNAME_LEN);
            hent = gethostbyname(hostname);
            std::cout<<"hostname:"<<hent->h_name<<" addr list:\n";
            for (int i = 0; hent->h_addr_list[i] != NULL; i++) {
                std::cout<<inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]))<<std::endl;
            }
            */

            return true;
        }
        void PrintHostList();
        bool GetFileList(std::string &path);
        void PrintFileList();
        uint64_t GetFileLength(std::string &file);
        bool DistributeDownLoad(std::string &url, uint64_t length);
        bool FileDownLoad(std::string &url, uint64_t start, uint64_t end);
    private:
        uint32_t _net_seg;
        uint32_t _start_host;
        uint32_t _end_host;
        ThreadPool *_tp;
        std::unordered_map<std::string, sockaddr_in> _host_list;
        std::unordered_map<std::string, std::string> _file_list;
};

int main() 
{
    Client client;
    client.StartClient();
    return 0;
}
