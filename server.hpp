#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include "cpp-httplib/httplib.h"
#include "utils.hpp"

#define BASE_DIR    "www"
using namespace httplib;
class P2PServer
{
    public:
        P2PServer(){
        }
        ~P2PServer(){
        }
        bool Start() {
            srv.set_error_handler(error_handler);
            srv.set_base_dir(BASE_DIR);
            srv.Get("/hostpair", PairHandler);
            srv.Get("/filelist", DirectoryHandler);
            srv.Get("/filelist/(.*)", DownloadHandler);
            srv.listen("0.0.0.0", 9000);
            return true;
        }
    private:
        static void error_handler(const Request& req, Response& res) {
            const char* fmt = "<h1>Error Status: <span style='color:red;'>%d</span></h1>";
            char buf[BUFSIZ];
            snprintf(buf, sizeof(buf), fmt, res.status);
            res.set_content(buf, "text/html");    
        }
        static void PairHandler(const Request& req, Response& res) {
            std::cout<<"server get an pair request :"<<req.method<<std::endl;
        }
        static void DirectoryHandler(const Request& req, Response& res) {
            std::string phys = BASE_DIR + req.path;
            if (access(phys.c_str(), F_OK) < 0) {
                mkdir(phys.c_str(), 0775);
            }
            if (Utils::GetRealpath(phys) == false) {
                return;
            }
            if (phys.find(BASE_DIR) == std::string::npos) {
                return;
            }

            std::stringstream body;
            struct dirent **p_dirent = NULL;
            int sub_file_num = scandir(phys.c_str(), &p_dirent, 0, alphasort);
            for (int i = 0; i < sub_file_num; i++) {
                std::string name;
                if (req.path[req.path.size() - 1] != '/') {
                    name = req.path + "/" + p_dirent[i]->d_name;
                }else {
                    name = req.path + p_dirent[i]->d_name;
                }
                body << name << "\r\n";
            }
            res.set_content(body.str(), "text/plain");
            if (p_dirent) {
                for (int i = 0; i < sub_file_num; i++) {
                    free(p_dirent[i]);
                }   
                free(p_dirent);
            } 
        }
        static void DownloadHandler(const Request& req, Response& res) {
            std::string phys = BASE_DIR + req.path;
            struct stat st;
            if (stat(phys.c_str(), &st) < 0) {
                res.status = 404;
                res.set_content("no such file or directory", "text/plain");
            }
            if (st.st_mode & S_IFDIR) {
                return DirectoryHandler(req, res);
            }
            if (req.method == "HEAD") {
                std::stringstream header;
                header << st.st_size;
                res.set_header("Content-Length", header.str().c_str());
            }else if(req.method == "GET") {
                if (!req.has_header("Range")) {
                    return;
                }
                int64_t range_start, range_end, range_size;
                std::string range = req.get_header_value("Range", 0);
                if (GetRange(range, range_start, range_end) == false) {
                    return ;
                }
                range_size = range_end - range_start + 1;
                if (range_start < 0 || range_end > st.st_size || range_end <= range_start) {
                    res.status = 400;
                    return ;
                }
                std::string body;
                body.resize(range_size);
                std::string path = BASE_DIR + req.path;
                std::ifstream fs(path, std::ios_base::binary);
                fs.seekg(range_start);
                fs.read(&body[0], range_size);
                res.set_content(body.c_str(), range_size, "text/plain");

                res.status = 206;
                res.set_header("Content-Length", std::to_string(range_size).c_str());
            
                return ;
            }
        }
        static bool GetRange(std::string &range, int64_t &range_start, int64_t &range_end) {
            size_t pos1, pos2;
            pos1 = range.find("-");
            if (pos1 == std::string::npos) return false;
            pos2 = range.find("bytes=");
            if (pos2 == std::string::npos) return false;
            std::stringstream ss;
            ss << range.substr(pos2 + 6, pos1 - pos2 - 6);
            ss >> range_start;
            ss.clear();
            ss << range.substr(pos1 + 1);
            ss >> range_end;
            return true;
        }
    private:
        httplib::Server srv;
};
