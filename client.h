#include <iostream>

class Client {
private:
    int file_size(int fd);
    int sock;
public:
    void create_and_connect(sockaddr_in* addr, int port);     
    void send_file(const char* s, int len, bool block, vector<char*> namev); 
    void send_msg(const char*s, int len, bool block, vector<char*> namev);
    void send_reg(const char* s, int len);
    void recieve(const char* s, int len);
    void shutconn();
};
