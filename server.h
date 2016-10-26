#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/epoll.h>

/* Data segments server recieved
 * msg:
 * type:1; flg:1; name size:4; name lists:some;
 * msg size:4; msg:some;
 *
 * file:
 * type:1; flg:1, name size:4; name lists:some;
 * file size:4; file-name-size:4; file-name:some;
 * file:some;
 *
 * reg:
 * type:1; name size:4; name:some;
 * */

/* Data segements client recieved
 * msg:
 * type:1; source-name-size:4; source-name:some;
 * msg-size:4; msg:some;
 * 
 * file:
 * type:1; source-name-size:4; source-name:some;
 * file-size:4; file-name-size:4; file-name:some;
 * file:some;
 * */

/*Attention:
 * All the 4 bytes size should include the last
 * '\0' in strings. Thus size should equal to 
 * strlen(str) + 1*/

#define MAX_EVENTS 100
#define MAX_THREAD_NUMS 10
const int maxnode = 1e3+10;
const int sigma_size = 26;
const int def_port = 8087;
const int max_file_sz = 1e6 + 10;
const int buf_size = 1024;

using namespace std;

struct Trie {
    int ch[maxnode][sigma_size];
    int val[maxnode], sz, fds[maxnode];
    inline void clear() { sz=1; memset(ch[0], 0, sizeof(ch[0]));}
    inline int idx(char c) { return c - '0';}
    void insert(char* s, int len, int fd);
    int find(char* s, int len);
    void del(char* s, int len);
};

class Server {
private:
    struct epoll_event ev, events[MAX_EVENTS];
    int lsnfd, connfd, nfds, epollfd;
    Trie trie;
    map<int, char*> fd2str;
    typedef int (Server::*Mfunc)();
private:
    int run_concurrent(Mfunc f);
    int setnonblocking(int fd);
    int file_size(int fd);
    int local_ip_address(struct sockaddr_in* res, int port);
    void process(int sock);
    int get_namelist(int sock, char* buf, int& offset, vector<int>& ans);
    int del_fdinfo(int fd);
    int delfd(int fd);
    int process_reg(int sock, char* buf, int buf_len, int offset);
    int process_msg(int sock, char* buf, int buf_len, int offset);
    int process_file(int sock, char* buf, int buf_len, int offset);
    int setup_listen();
    int monitor();
public:
    int start();
    int stop();
};

