#ifndef _COWR_SELECTOR

#define _COWR_SELECTOR

#include <sys/select.h>

#include <functional>
#include <map>

class Selector {
private:
    fd_set read_set, write_set, err_set;
    std::map<int, std::function<void(void)>> handlers;
};

#endif
