#include <iostream>
#include "zookeeper.h"
#include "zookeeperplus.h"

void watcher_func(zhandle_t *zh, int type,
                  int state, const char *path, void *watcherCtx) {

}

int main() {
    std::cout << "Hello, World!" << std::endl;
    zhandle_t *handler = zookeeper_init("0.0.0.0:2181", watcher_func,
                                        1000, NULL, NULL, 0);
    return 0;
}