//
// Created by wangchun on 2016/11/12.
//

#ifndef CURATORPLUS_ZOOKEEPER_GUARD_H
#define CURATORPLUS_ZOOKEEPER_GUARD_H

#include <string>
#include <vector>

class zookeeper_guard {
public:
    zookeeper_guard();
    ~zookeeper_guard();

    // 初始化
    int init();

    // 设置是否自动重连
    void auto_reconnect(bool);

    // 创建路径
    int create_full_path(string path);

    // 创建节点
    string create_node(string path, int flag);

    // 获取子节点
    vector<string> get_children(string path);

    // 获取并监听节点
    string get_and_watch(string path, watcher_fn func, void* cbdata);

protected:


};

#endif //CURATORPLUS_ZOOKEEPER_GUARD_H
