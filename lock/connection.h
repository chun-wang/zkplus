//
// Created by wangchun on 2016/11/12.
//

#ifndef CURATORPLUS_CONNECTION_H
#define CURATORPLUS_CONNECTION_H

#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "zookeeper.h"

using namespace std;

const uint32_t EVENT_CONNECTION_ON = (1 << 0);
const uint32_t EVENT_CONNECTION_OFF = (1 << 1);

class connection_listener {
public:
    virtual ~connection_listener(){}

    void on_event(uint32_t event);
};

class connection {
    string _server;
    bool _connected;
    zhandle_t* _handler;
    condition_variable _cond_val;
    vector<connection_listener*> _listeners;
    unique_lock _mutex;
public:
    connection(std::string serv);
    ~connection();

    int connect();

    int connect(chrono::milliseconds timeout);

    int close();

    bool is_connected() { return _connected; }

    int add_listener(connection_listener* listener);

    int remove_listener(connection_listener* listener);

protected:
    static void watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

    void proc_session_event(zhandle_t *zh, int state);

    void notify_listeners(uint32_t event);
};


#endif //CURATORPLUS_CONNECTION_H
