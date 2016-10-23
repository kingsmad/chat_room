#include <iostream>

struct Trie {
    int ch[maxnode][sigma_size];
    int val[maxnode], sz;
    inline void clear() { sz=1; memset(ch[0], 0, sizeof(ch[0]));}
    inline int idx(char c) { return c - '0';}
    void insert(char* s, int len);
    int find(char* s, int len);
    void del(char* s, int len);
};
