#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include "server.hpp"
#include "client.hpp"
#define SERVER_PORT 9000
void *thr_server(void *arg)
{
    P2PServer srv;
    srv.Start();
    return NULL;
}
bool Start()
{
    pthread_t srv_tid;
    int ret;
    ret = pthread_create(&srv_tid, NULL, thr_server, NULL);
    if (ret != 0) {
        std::cout<<"thread create error\n";
        return false;
    }
    pthread_detach(srv_tid);

    P2PClient cli;
    cli.Start();
    return true;
}

void chld_sigcb(int signo) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
void pipe_sigcb(int signo) {
}

void SigInit()
{
    signal(SIGCHLD, chld_sigcb);
    signal(SIGPIPE, pipe_sigcb);
}
int main()
{
    SigInit();
    Start();
    return 0;
}
