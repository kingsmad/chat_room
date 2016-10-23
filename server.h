#include <iostream>
#include <map>
#define MAX_EVENTS 100
#define MAX_THREAD_NUMS 10

class Server {
private:
    ThreadPool thdpool(10);
    struct epoll_event ev, events[MAX_EVENTS];
    int lsnfd, connfd, nfds, epollfd;
    Trie trie;
    map<int, char*> fd2str;
    thread th_monitor;
public:
    void local_wifi_ipaddress(char* s);
    int setup_listen();
    void process(int sock);
    int get_namelist(int sock, char* buf, int buf_len, int offset, vector<int>& ans);
    int del_fdinfo(int fd);
    int delfd(int fd);
    int process_reg(int sock, char* buf, int buf_len, int offset);
    int process_msg(int sock, char* buf, int buf_len, int offset);
    int process_file(int sock, char* buf, int buf_len, int offset);
    int start();
};

