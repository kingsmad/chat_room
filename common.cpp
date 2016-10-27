/*const int MAX_EVENTS = 100;
const int MAX_THREAD_NUMS = 10;
const int maxnode = 1e3+10;
const int sigma_size = 26;
const int def_port = 8087;
const int max_file_sz = 1e6 + 10;
const int buf_size = 1024;*/

const int debug = true;
void buginfo(const char* f, ...) {if(!debug)return;va_list al; va_start(al, f);vprintf(f, al);va_end(al);}
