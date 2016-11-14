//
// Created by wangchun on 2016/11/12.
//

#ifndef CURATORPLUS_ZOOKEEPER_LOCK_H
#define CURATORPLUS_ZOOKEEPER_LOCK_H

#include "zookeeper.h"
#include "zookeeper_guard.h"
#include <string>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>

using namespace std;

class lock_listener {
public:
    virtual ~lock_listener(){}

    virtual void on_event(uint32_t event) = 0;
};

class zookeeper_lock;

class lock_future {
    int _result;
    string _id;
    bool _is_valid;
    bool _is_setted;
    lock_listener* _listener;
    condition_variable _cond;
    unique_lock _mutex;
    zookeeper_lock* _locker;
public:
    lock_future(zookeeper_lock* locker) {
        _is_valid = true;
        _locker = locker;
    }

    void set(int result);

    int get();

    int get(chrono::milliseconds timeout);

    int add_listener(lock_listener* listener);

    void set_valid(bool is_valid);

    bool is_valid() { return _is_valid; }

    void set_id(string id) { _id = id; }

    string get_id() { return _id; }

    zookeeper_lock* get_zookeeper_lock() {
        return _locker;
    }
};

class zookeeper_lock {
    zhandle_t *_handler;
    string _path;
    string _lock_tag;
    string _id;
    string _owner_id;
    lock_listener* _listener;
    condition_variable _cond_val;
    unique_lock _mutex;
    atomic_uint _recursive_count;
    zookeeper_guard _client;
public:

    zookeeper_lock(zhandle_t* handler, string path, string tag="");
    ~zookeeper_lock();

    int sync_lock();

    int async_lock();

    int timed_lock(chrono::milliseconds timeout);

    int unlock();

    bool is_owner();

protected:

    static void lock_watcher_fn(zhandle_t* zh, int type, int state, const char* path, void *watcherCtx);

    void lock_operation(lock_future* future);

    string get_node_prefix(const string node);

    string get_node_id(const string node);
};


#endif //CURATORPLUS_ZOOKEEPER_LOCK_H
