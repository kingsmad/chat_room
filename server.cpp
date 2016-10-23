#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "server.h"

#define MAX_EVENTS 20
const int max_file_sz = 1e6 + 10;

using namespace std;

void Trie::insert(char* s, int len, int fd) {
    int u = 0;
    for(int i=0; i<len; ++i) {
        int c = idx(s[i]);
        if (!ch[u][c]) {
            memset(ch[sz], 0, sizeof(ch[sz]));
            ch[u][c] = sz++;
        }
        u = ch[u][c];
        ++val[u];
    }
    fds[u] = fd; 
}

int Trie::find(char* s, int len) {
    int t = 0, u = 0;
    for(int i=0; i<len; ++i) {
        int c = idx(s[i]);
        if (!ch[u][c]) return -1;
        u = ch[u][c];
    }
    return fds[u];
}

void Trie::del(char* s, int len) {
    int u = 0;
    for(int i=0; i<len; ++i) {
        int c = idx(s[i]);
        int tu = ch[u][c]; 
        if (--val[ch[u][c]]<=0) ch[u][c]=0;
        u = tu;
    }
    fds[u] = -1;
}

int Server::start() {
    // re-init, close fds
    if (lsnfd) close(lsnfd);
    if (connfd) close(connfd);
    if (nfds) close(nfds);
    if (epollfd) close(epollfd);

    // clear db
    trie.clear();
    fd2str.clear();

    setup_listen();    
    th_monitor = thread(monitor);

    return 0;
}

int Server::setnonblocking(int fd) {
    int flg, s;
    flg = fcntl(fd, F_GETFL, 0);
    if (flg == -1) {
        buginfo("Fuck you.\n");
        return -1;
    }
    flg |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flg) == -1) {
        buginfo("Fuck you fcntl.\n");
    }

    return 0;
}

int Server::file_size(int fd) {
    struct stat s;
    if (fstat(fd, &s) == -1) {
        buginfo("\nfstat error;\n");
        return -1;
    }
    return s.st_size;
}

int Server::local_ip_address(struct sockaddr_in* res, int port) {
    struct ifaddrs* addrs;
    getifaddrs(&addrs);
    
    struct sockaddr_in* cur_addr = NULL;
    for (struct ifaddrs* p=addrs; p->ifa_next; p=p->ifa_next;) {
        if (p->ifa_addr->sa_family==AF_INET && strcmp(p->ifa_name, "en0")) {
            cur_addr = (struct sockaddr_in*) p->ifa_addr;
        }
    }

    if (cur_addr == 0) return -1;

    cur_addr->sin_port = htons(port);
    cur_addr->sin_family = AF_INET;
    *res = cur_addr;

    freeifaddrs(addrs);
    return 0;
}

int Server::setup_listen () {
    if (lsnfd != 0) close(lsnfd);

    lsnfd = socket(AF_INET, SOCK_STREAM, 0);
    // set reusable
    optval = 1;
    setsockopt(lsnfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)); 

    // get ip-address
    struct sockaddr_in sv_addr;
    if (local_ip_address(&sv_addr, def_port) < 0) 
        printf("\nFailed to get IP address, check your network!\n");
        return -1;
    }

    if (bind(lsnfd, (struct sockaddr*) &sv_addr, sizeof(sv_addr)) < 0) {
        printf("\n Error in Binding lsnfd...\n");
        return -1;
    }

    if (listen(lsnfd, 10) < 0) {
        buginfo("\nFailed in listening...\n");
        return -1;
    }

    return 0;
}

// monitor the fds, process incoming data
int Server::monitor () {
    epollfd = epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = lsnfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lsnfd, &ev) == -1 ) {
        buginfo("\nepoll_ctl, lsnfd failed\n.");
        return -1;
    }

    while(true) {
        nfds = wait(epollfd, events, MAX_EVNETS, -1);
        for(i=0; i<nfds; ++i) {
            if ((events[i].events & EPOLLERR) || (events[i].events \
                    & EPOLLHUP)) {
                /*In our case, this must be sth wrong*/
                buginfo("epoll error\n");
                close(events[i].data.fd);
                delfd(events[i].data.fd);
                continue;
            }
            if (events[i].data.fd == lsnfd) { // new connection coming
                struct sockaddr_in addr;
                int len = sizeof(sockaddr_in);
                connfd = accept(lsnfd, (struct sockaddr_in*)&addr, &len); 
                if (connfd == -1) {
                    buginfo("accepted wrong connfd...\n");
                    return -1;
                }
                setnonblocking(connfd); //ET needs non-blocking
                ev.events = EPOLL | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    buginfo("Config error in connfd...\n");
                    return -1;
                }
            } else {
                process(events[i].data.fd);
            }
        }
    }
}

// do processing while new data coming.
void Server::process(int sock) {
    // extract header;
    char buf = (char*)malloc(1024);
    memset(buf, 0, sizeof(buf));
    int offset = 0;

    while(offset < 1) 
        offset += read(sock, buf+offset, 1-offset);
    
    assert(offset == 1);
    if (buf[0] == 0) 
        process_reg(sock, buf, sizeof(buf), offset);
    else if (buf[0] == 1) 
        process_msg(sock, buf, sizeof(buf), offset);
    else 
        process_file(sock, buf, sizeof(buf), offset);
    free(buf);
}

int Server::get_namelist(int sock, char* buf, int& offset, vector<int>& ans) {
    // read flg && name-strings-size: 5 bytes total 
    int flgst = offset;
    int szst = offset+1;
    while(offset < flgst+5)
        offset += read(sock, buf+offset, flgst+5-offset);
    uint32_t sz = 0;
    memcpy(&sz, buf+szst, 4);
    sz = ntohl(sz);
    int flg = buf[flgst]; // flg indicates whether it's blacklist

    // read name strings 
    int strst = offset;
    while(offset < strst+sz) 
        offset += read(sock, buf+offset, strst+sz-offset);
    
    // query from trie, get fds
    set<int> tfds;
    for(int i=strst; i<strst+sz; ) {
        int j = i+1;
        while(j<strst+sz && buf[j]!=0) ++j;
        int cfd = trie.find(buf+i, j-i); 
        if (cfd > 0) tfds.insert(cfd);
    }
        
    // write ans
    ans.clear();
    if (flg == 0) {
        ans.resize(tfds.size());
        copy(tfds.begin(), tfds.end(), ans.begin());
    } else if(flg == 1) {
        for (auto c=fdset.begin(); c!=fdset.end(); ++c) {
            if (tfds.count(*c) > 0) continue;
            ans.push_back(*c);
        }
    }
      
    return 0;
}

int Server::del_fdinfo(int fd) {
    if (fd2str.count(sock) == 0) return 0;
    auto c = fd2str.find(sock);
    char* p = c->second;
    trie.del(p, strlen(p)); 
    fd2str.erase(c);
}

int Server::delfd(int fd) {
    del_fdinfo(fd);
    close(fd);
}

int Server::process_reg(int sock, char* buf, int buf_len, int offset) {
    // read name size
    int naszst = offset;
    while(offset < naszst+4) 
        offset += read(sock, buf+offset, naszst+4-offset);
    uint32_t nasz = 0;
    copy(buf+naszst, buf+naszst+4, (char*)&nasz);
    nasz = ntohl(nasz);
    
    // read name string
    int nast = offset;
    while(offset < nast + nasz) // nasz included the last '\0'
        offset += read(sock, buf+offset, nast+nasz-offset); 
    
    // check if this is a rename
    if (fd2str.count(sock) > 0) {
        del_fdinfo(sock);
    }

    // update information
    trie.insert(buf+nast, nasz, sock);
    char* str = (char*) malloc(nasz);
    copy(buf+nast, buf+nast+nasz, str);
    fd2str.insert(make_pair(sock, str));
   
    return 0;
}

int Server::process_msg(int sock, char* buf, int buf_len, int offset) {
    vector<int> name_fds;
    get_namelist(sock, buf, buf_len, offset, name_fds); 

    // msg size
    int msgszst = offset;
    while(offset < msgszst+4)
        offset += read(sock, buf+offset, msgszst+4-offset);
    unit32_t tsz = 0;
    memcpy(&tsz, buf+msgszst, 4);
    tsz = ntohl(tsz);

    // read all
    int msgst = offset;
    while(offset < msgst+tsz) 
        offset += read(sock, buf+offset, msgst+tsz-offset);

    // send msg to net.
    for(auto fd: name_fds) {
        write(fd, buf, 1); // 1 byte type 

        // add sender's name
        char* p = fd2str[sock];
        int plen = strlen(p)+1; // inlcude last '\0'
        int a = htonl(plen);
        write(fd, &a, 4);  
        write(fd, p, plen);

        write(fd, msgst, 4);
        write(fd, buf+msgst, tsz); 
    }
}

int Server::process_file(int sock, char* buf, int buf_len, int offset) {
    vector<int> name_fds;
    get_namelist(sock, buf, buf_len, offset, name_fds);

    // file size and its' name-size
    int fszst = offset;
    offset += read(sock, buf+offset, 8);
    uint32_t tsz = 0, nasz = 0;
    copy(buf+fszst, buf+fszst+4, (char*)tsz);
    copy(buf+fszst+4, buf+fszst+8, (char*)nasz);
    tsz = ntohl(tsz);
    nasz = ntohl(nasz);

    // get file-name
    int namest = offset;
    while(offset < namest+nasz)
        offset += read(sock, buf+offset, namest+nasz-offset); 

    // save file
    int filest = offset;
    int file = open(buf+namest, O_CREAT | O_TRUNC | O_WRONLY);
    if (file < 0) {
        buginfo("\n Failed to open file: %s\n", buf+namest);
        return -1;
    }
    sendfile(file, sock, 0, tsz);
    close(file);

    // assemble segment && send file
    file = open(buf+namest, O_RDONLY, 0644);
    assert(tsz == file_size(file));
    if (file < 0) {
        buginfo("\n Re-open file %s error!\n", buf+namest);
        return -1;
    }
    for (int fd: name_fds) { // can be concurrent in future;
        optval = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));
         
        write(fd, buf, 1); // 1 byte type

        // add sender's name
        char* p = fd2str[sock];
        assert(p!=0);
        int plen = strlen(p)+1;
        int a = htonl(plen);
        write(fd, &a, 4); // source-name-size 
        write(fd, p, plen); // source-name

        // filesz 4 bytes and filename
        write(fd, buf+fszst, filest-fszst); 

        // file content
        int out_offset = 0;
        sendfile(fd, file, &out_offset, tsz); 

        optval = 0;
        setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));
    }
}

