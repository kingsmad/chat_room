#include <iostream>
#include <vector>
using namespace std;

class Client {
private:
    int sock;
private:
    int file_size(int fd);
    int set_uint32(char* s, int k);
    int get_uint32(char* s);
    void set_namelist(char* buf, int& offset, vector<char*>& namev);
    void process_file(char* buf, int offset);
    void process_msg(char* buf, int offset);
public:
    int create_and_connect(const char* s, int len, int port);
    void send_file(const char* s, int len, bool block, vector<char*> namev); 
    void send_msg(const char*s, int len, bool block, vector<char*> namev);
    void send_reg(const char* s, int len);
    void monitor();
    void shutconn();
};
