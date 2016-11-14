//
// Created by wangchun on 2016/11/12.
//

#include "connection.h"
#include <cassert>
#include <chrono>
#include <algorithm>

connection::connection(string serv) {
    _connected = false;
    _server = serv;
    _handler = NULL;
}

connection::~connection() {

}

int connection::connect() {
    // lock the world
    lock_guard<unique_lock> auto_lock(_mutex);
    if (_handler != NULL) {
        return -1;
    }

    _handler = zookeeper_init("0.0.0.0:2181", connection::watcher, 1000, NULL, NULL, 0);
    if (_handler == NULL) {
        return -1;
    }

    return 0;
}

int connection::connect(chrono::milliseconds timeout) {
    // lock the world
    lock_guard<unique_lock> auto_lock(_mutex);
    if (_handler != NULL && _connected) {
        return -1;
    }

    _handler = zookeeper_init("0.0.0.0:2181", connection::watcher, 1000, NULL, NULL, 0);
    if (_handler == NULL) {
        return -1;
    }

    bool ret = _cond_val.wait_for(_mutex, timeout, []{return _connected;});
    if (ret == false) {
        return -1;
    }

    return 0;
}

int connection::close() {
    lock_guard<unique_lock> auto_lock(_mutex);

    return zookeeper_close(_handler);
}

int connection::add_listener(connection_listener* listener) {
    lock_guard<unique_lock> auto_lock(_mutex);

    auto it_find = find(_listeners.begin(), _listeners.end(), listener);
    if (it_find != _listeners.end()) {
        // already exist
        return -1;
    }

    _listeners.push_back(listener);
    return 0;
}

int connection::remove_listener(connection_listener *listener) {
    lock_guard<unique_lock> auto_lock(_mutex);

    auto it_find = find(_listeners.begin(), _listeners.end(), listener);
    if (it_find == _listeners.end()) {
        // not exist
        return -1;
    }

    _listeners.erase(it_find);
    return 0;
}

void connection::watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {
    assert(watcherCtx != NULL);

    connection* self = static_cast<connection*>(watcherCtx);
    switch (type) {
        case ZOO_SESSION_EVENT:
            self->proc_session_event(zh, state);
            break;
        default:
            break;
    }
}

void connection::proc_session_event(zhandle_t *zh, int state) {
    lock_guard<unique_lock> auto_lock(_mutex);
    if (state == ZOO_CONNECTED_STATE) {
        _connected = true;
        _cond_val.notify_all();
        notify_listeners(EVENT_CONNECTION_ON);
    } else if (state == ZOO_AUTH_FAILED_STATE || state == ZOO_EXPIRED_SESSION_STATE) {
        _connected = false;
        _cond_val.notify_all();
        notify_listeners(EVENT_CONNECTION_OFF);
    } else {

    }
}

void connection::notify_listeners(uint32_t event) {
    for_each(_listeners.begin(), _listeners.end(), [](connection_listener* listener){
        listener->on_event(event);
    });
}