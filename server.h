#include <iostream>
#include <map>
#define MAX_EVENTS 100
#define MAX_THREAD_NUMS 10

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
 * file-size:4; file:some;
 * */

/*Attention:
 * All the 4 bytes size should include the last
 * '\0' in strings. Thus size should equal to 
 * strlen(str) + 1*/

struct Trie {
    const int maxnode = 1e3+10;
    const int sigma_size = 26;
    int ch[maxnode][sigma_size];
    int val[maxnode], sz, fds[maxnode];
    inline void clear() { sz=1; memset(ch[0], 0, sizeof(ch[0]));}
    inline int idx(char c) { return c - '0';}
    void insert(char* s, int len);
    int find(char* s, int len);
    void del(char* s, int len);
};

class Server {
    const int def_port = 8087;
    private:
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

