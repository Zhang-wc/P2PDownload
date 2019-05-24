#include <netdb.h>
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
            char hostname[HOSTNAME_LEN];
            gethostname(hostname, HOSTNAME_LEN);
            std::cout<<hostname<<std::endl;

            struct hostent *hent;

            hent = gethostbyname(hostname);
            std::cout<<"hostname:"<<hent->h_name<<" addr list:\n";

            for (int i = 0; hent->h_addr_list[i] != NULL; i++) {
                std::cout<<inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]))<<std::endl;
            }
            return true;
        }
        void PrintHostList();
        bool GetFileList(std::string &path);
        void PrintFileList();
        uint64_t GetFileLength(std::string &file);
        bool DistributeDownLoad(std::string &url, uint64_t length);
        bool FileDownLoad(std::string &url, uint64_t start, uint64_t end);
    private:
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
