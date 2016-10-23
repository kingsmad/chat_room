#include <iostream>
#include <client.h>
#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;


int Client::file_size(int fd) {
    sruct stat s;
    if (fstat(fd, &s) == -1) {
        buginfo("\nfstat returned erro;");
        return -1;
    }
    return s.st_size;
}

int Client::create_and_connect(const char* s, int len, int port) {
    // create
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    inet_pton(AF_INET, s, &serveraddr.in_addr);
    serveraddr.sin_port = htonl(port);
    serveraddr.sin_family = AF_INET;

    // connect
    if (connect(sock, &serveraddr, sizeof(serveraddr)) < 0) {
        buginfo("Connect Failed!\n");
        return -1;
    }
}

int Client::send_file(const char* s, int len, bool block, vector<char*> namev) {
    /*The header of the file-msg is fixed to be 
     * less than 1024*/
    char buf = (char*)malloc(1024);
    offset = 0;
    memset(buf, 0, sizeof(buf));
    // type
    buf[offset++] = 2;
    
    // size of name-lists
    int naszst = offset;
    union {
        char b[4];
        uint32_t d;
    } tmp;

    // name-lists and fill size back
    int nvst = offset;
    for (char* c: namev) {
        int tlen = strlen(c);
        copy(c, c+tlen, buf+offset);
        offset += tlen+1; // reserver 0 at back
    }
    tmp.d = htonl(offset-nvst);
    copy(tmp.b, tmp.b+4, buf+offset);
    offset += 4;
    
    // add file-name-sz and file-name
    tmp = 0;
    tmp.d = len+1;
    copy(tmp.b, tmp.b+4, buf+offset);
    offset += 4;
    copy(s, s+len, buf+offset);
    offset += len+1;

    // deal with file
    int file = open(s, O_RDONLY, 0644);
    if (file < 0) {
        buginfo("File not exist...\n");
        return -1;
    }
    int file_sz = file_size(file);
    
    // send to network
    int optval = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));

    write(sock, buf, offset); 
    sendfile(sock, file, 0, file_sz);

    optval = 0;
    setsockopt(sock, IPPROTO_TCP, TCP_CORK, &optval, sizeof(int));

    free(buf);
}


void Client::send_msg(const char* s, int len, bool block, vector<char*> namev) {
    /*The header of the file-msg is fixed to be 
     * less than 1024*/
    char* buf = (char*)malloc(1024*2);
    offset = 0;
    memset(buf, 0, sizeof(buf));
    // type
    buf[offset++] = 1;
    
    // size of name-lists
    int naszst = offset;
    union {
        char b[4];
        uint32_t d;
    } tmp;

    // name-lists and fill size back
    int nvst = offset;
    for (char* c: namev) {
        int tlen = strlen(c);
        copy(c, c+tlen, buf+offset);
        offset += tlen+1; // reserver 0 at back
    }
    tmp.d = htonl(offset-nvst);
    copy(tmp.b, tmp.b+4, buf+1);
    offset += 4;
    
    // add msg    
    copy(s, s+len+1, buf+offset);
    offset += len+1;

    // send to network
    write(sock, buf, offset);

    free(buf);
}

void Client::send_reg(const char* s, int len) {
    char* buf = (char*) malloc(100);
    int offset = 0;

    // type
    buf[offset++] = 0;
    
    // name sz
    union {
        char b[4];
        uint32_t d;
    } tmp;
    tmp.d = htonl(len+1);
    copy(tmp.d, tmp.d+4, buf+offset);
    offset += 4;

    // send
    write(sock, buf, offset);
}

void Client::monitor() {
    char* buf = (char*)malloc(1024);
    int offset;
    while(true) {
        offset = 0;
        memset(buf, 0, sizeof(buf));
        offset += read(sock, buf, 1); 
        if (buf[0] == 1) { // it's message
            read(sock, buf, 4);  
            int sz;
            memcpy(&sz, buf+1, 4);
            sz = ntohl(sz);
            while(offset < 1+4+sz) 
                offset += read(sock, buf+offset, 1+4+sz-offset);
            printf("Got Message: \n%s\n", buf+1+4);      
        } else if (buf[1] == 2) { // it's file
            int fszst = offset;
            offset += read(sock, buf+offset, 8);
            uint32_t file_sz, na_sz;
            copy(fszst, fszst+4, (char*)&file_sz);
            copy(fszst+4, fszst+8, (char*)&na_sz);
            file_sz = ntohl(file_sz);
            na_sz = ntohl(na_sz);

            // get file-name
            int nast = offset;
            while(offset < nast+na_sz)
                offset += read(sock, buf+offset, nast+na_sz-offset);

            // save file to local
            int ffd = open(buf+nast, O_CREATE | O_TRUNC);
            sendfile(ffd, sock, 0, file_sz);
        }
    }

    free(buf);
}

void Client::shutconn() {
    close(sock);
}
