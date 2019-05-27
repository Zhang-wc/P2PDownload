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
            req.SetMethod("GET");
            req.SetPath("/hostpair");
            req.SetVersion("HTTP/1.1");
            req.SetHeader("Content-Length", "0");
            req.SetHeader("Connection", "close");
            //所有主机号中全0和全1不能使用
            for (int i = 0; i < host_list.size(); i++){
                std::cout<<host_list[i]<<std::endl;
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
        void PrintHostList();
        bool GetFileList(std::string &path);
        void PrintFileList();
        uint64_t GetFileLength(std::string &file);
        bool DistributeDownLoad(std::string &url, uint64_t length);
        bool FileDownLoad(std::string &url, uint64_t start, uint64_t end);
        void SetSharedPath(std::string &path);
    private:
        std::string _shared_path;
        ThreadPool *_tp;
        std::vector<std::string> _host_list;
        std::unordered_map<std::string, std::string> _file_list;
};

int main() 
{
    Client client;
    client.StartClient();
    return 0;
}
