//
// Created by wangchun on 2016/11/12.
//

#include "zookeeper_lock.h"
#include <algorithm>
#include <cassert>

using namespace std;

lock_future::lock_future(zookeeper_lock *locker) {
    _listener = NULL;
    _is_valid = true;
    _is_setted = false;
    _locker = locker;
    _result = -1;
}

void lock_future::set(int result) {
    lock_guard<unique_lock> locker(_mutex);
    _result = result;
    if (_listener) {
        _listener->on_event(_result);
    }

    _cond.notify_all();
}

int lock_future::get() {
    lock_guard<unique_lock> locker(_mutex);
    while (!_is_setted) {
        _cond.wait(_mutex);
    }

    return _result;
}

int lock_future::get(chrono::milliseconds timeout) {
    auto start = chrono::steady_clock::now();
    auto left_time = timeout;

    lock_guard<unique_lock> locker(_mutex);
    while (!_is_setted && left_time.count() > 0) {
        _cond.wait_for(_mutex, left_time);

        left_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
    }

    return _result;
}

int lock_future::add_listener(lock_listener *listener) {
    lock_guard<unique_lock> locker(_mutex);
    _listener = listener;
}

void lock_future::set_valid(bool is_valid) {
    lock_guard<unique_lock> locker(_mutex);
    _is_valid = is_valid;
}

zookeeper_lock::zookeeper_lock(zhandle_t* handler, string path, string tag="") {
    _handler = handler;
    _path = path;
    _lock_tag = "lock-" + tag;
    _id = 0;
    _owner_id = 0;
    _listener = NULL;
    _recursive_count = 0;
}

zookeeper_lock::~zookeeper_lock() {

}

int zookeeper_lock::sync_lock() {

}

int zookeeper_lock::async_lock() {


}

int zookeeper_lock::timed_lock(chrono::milliseconds timeout) {

}

int zookeeper_lock::unlock() {

}

void zookeeper_lock::lock_operation(lock_future* future) {
    lock_guard<unique_lock> locker(_mutex);

    do {
        // 创建本节点
        if (_id.empty()) {
            string fullpath = _path + _lock_tag + "-";
            string created = _client.create_node(fullpath, ZOO_EPHEMERAL|ZOO_SEQUENCE);
            if (created.empty()) {
                future->set(-1);
                break;
            }

            _id = get_node_id(created);
            future->set_id(_id);
        }

        // 检查是否为leader节点
        if (!_id.empty()) {
            auto children = _client.get_children(_path);
            sort(children.begin(), children.end());
            _owner_id = get_node_id(*children.begin());

            auto index = find(children.begin(), children.end(), _lock_tag+_id);
            // we got the lock
            if (index == children.begin()) {
                // set future info
                future->set(0);
                break;
            }

            // system error
            if (index == children.end()) {
                future->set(-1);
                break;
            }

            // watch the previous node
            string pre_node = _path + *(--index);
            string watched = _client.get_and_watch(pre_node, zookeeper_lock::lock_watcher_fn, future);
            if (watched.empty()) {
                // 前一节点已经被删除，重新检查节点顺序
                continue;
            }

            break;
        }
    } while(true);
}

string zookeeper_lock::get_node_prefix(const string node) {
    size_t pos = node.rfind('-');
    if (pos == string::npos) {
        return "";
    }

    return node.substr(0, pos);
}

string zookeeper_lock::get_node_id(const string node) {
    size_t pos = node.rfind('-')+1;
    if (pos == string::npos) {
        return "";
    }

    return node.substr(pos, node.size()-pos);
}

void zookeeper_lock::lock_watcher_fn(zhandle_t* zh, int type, int state, const char* path, void *watcherCtx) {
    assert(watcherCtx != NULL);
    lock_future* future = static_cast<lock_future*>(watcherCtx);
    if (future->is_valid()) {
        zookeeper_lock* self = future->get_zookeeper_lock();
        lock_guard<unique_lock> locker(self->_mutex);
        if (self->_id == future->get_id()) {
            future->get_zookeeper_lock()->lock_operation(future);
        } else {
            delete future;
        }
    } else {
        delete future;
    }
}
