#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/epoll.h>
#include "common.h"

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


using namespace std;


class Trie;
class Server {
private:
    struct epoll_event ev, events[MAX_EVENTS];
    int lsnfd, connfd, nfds, epollfd;
    Trie* trie;
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
    Server();
    ~Server();
    int start();
    int stop();
};

