#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "server.h"

#define MAX_EVENTS 20
const int max_file_sz = 1e6 + 10;

using namespace std;

int start() {
    // re-init
    if (lsnfd) close(lsnfd);
    if (connfd) close(connfd);
    if (nfds) close(nfds);
    if (epollfd) close(epollfd);
    trie.init();
    fd2str.clear();
    for(int td: 

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
int Server::local_ip_address(struct sockaddr_in* res, int port) {
    bool ok;
    struct ifaddrs* addrs;
    getifaddrs(&addrs);
    
    struct sockaddr_in* cur_addr = NULL;
    for (struct ifaddrs* p=addrs; p->ifa_next; p=p->ifa_next;) {
        if (p->ifa_addr->sa_family==AF_INET && strcmp(p->ifa_name, 'en0')) {
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
    optval = 1;
    setsockopt(lsnfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)); 

    struct sockaddr_in sv_addr;
    local_ip_address(sv_addr, 8080);
    if (res == -1) {
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
                del(events[i].data.fd);
                continue;
            }
            if (events[i].data.fd == lsnfd) {
                struct sockaddr_in addr;
                int len = sizeof(sockaddr_in);
                connfd = accept(lsnfd, (struct sockaddr_in*)&addr, &len); 
                if (connfd == -1) {
                    buginfo("accepted wrong connfd...\n");
                    return -1;
                }
                setnonblocking(connfd);
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
}

int Server::get_namelist(int sock, char* buf, int buf_len, int& offset, vector<int>& ans) {
    // size of namestrings 
    int szst = offset;
    while(offset < szst+4)
        offset += read(sock, buf+offset, szst+4-offset);
    uint32_t sz = 0;
    memcpy(&sz, buf+szst, 4);
    sz = ntohl(sz);

    // name fd lists
    int strst = offset;
    while(offset < strst+sz) 
        offset += read(sock, buf+offset, strst+sz-offset);
    
    set<int> tfds;
    for(int i=strst; i<strst+sz; ) {
        int j = i+1;
        while(j<strst+sz && buf[j]!=0) ++j;
        int cfd = trie.query(buf+strst, sz); 
        if (cfd > 0) tfds.insert(cfd);
    }
        
    // write ans
    ans.clear();
    if (buf[1] == 0) {
        ans.resize(tfds.size());
        copy(tfds.begin(), tfds.end(), ans.begin());
    } else if(buf[1] == 1) {
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
    int st = 6, lst = st;
    while(buf[lst] != 0) ++lst;

    // check if this is a rename
    if (fd2str.count(sock) > 0) {
        del_fdinfo(sock);
    }

    // update information
    trie.insert(buf+st, lst-st, sock);
    char* str = (char*)malloc(lst-st+1);
    copy(buf+st, lst-st+1, str);
    fd2str.insert(make_pair(sock, str));  

    return 0;
}

int Server::process_msg(int sock, char* buf, int buf_len, int offset) {
    vector<int> name_fds;
    get_namelist(sock, buf, buf_len, offset, name_fds); 

    // data size
    unit32_t tsz = 0;
    read(sock, (char*)&tsz, 4);
    tsz = ntohl(tsz);
    offset += 4;

    // read all
    int msgst = offset;
    while(offset < msgst+tsz) 
        offset += read(sock, buf+offset, msgst+tsz-offset);

    // send msg to net.
    for(auto fd: name_fds) {
        write(fd, buf, 1); // 1 byte type 

        // add sender's name
        char* p = fd2str[sock];
        int plen = strlen(p);
        int a = htonl(plen);
        write(fd, &a, 4);  
        write(fd, p, plen+1);

        write(fd, msgst, 4);
        write(fd, buf+msgst, tsz); 
    }
}

int Server::process_file(int sock, char* buf, int buf_len, int offset) {
    int segst = offset;
    vector<int> name_fds;
    get_namelist(sock, buf, buf_len, offset, name_fds);

    // file size and its' name-size
    int fszst = offset;
    offset += read(sock, buf+offset, 8);
    uint32_t tsz = 0, nasz = 0;
    copy(buf+segst, buf+segst+4, (char*)tsz);
    copy(buf+segst+4, buf+segst+8, (char*)nasz);
    tsz = ntohl(tsz);
    nasz = ntohl(nasz);

    // get file-name
    int namest = offset;
    while(offset < buf+namest+nasz)
        offset += read(sock, buf+offset, buf+namest+nasz-offset); 

    // save file
    int filest = offset;
    while( offset < filest+tsz) 
        offset += read(sock, buf+offset, filest+tsz-offset);
    int file = open(buf+namest, O_CREAT | O_TRUNC);
    write(file, buf+filest, tsz);

    // send file
    for (int fd: name_fds) {
        optval = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));
         
        write(fd, buf, 1); // 1 byte type
        // add sender's name
        char* p = fd2str[sock];
        int plen = strlen(p);
        int a = htonl(plen);
        write(fd, &a, 4);  
        write(fd, p, plen+1);

        // filesz 4 bytes and filename
        write(fd, buf+fszst, filest-fszst); 
        // file content
        write(fd, buf+filest, offset-filest);

        optval = 0;
        setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));
    }
}

