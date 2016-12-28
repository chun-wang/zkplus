//
// Created by w00227109 on 2016/12/28.
//

#ifndef ZOOKEEPERPLUS_ZOOKEEPER_H
#define ZOOKEEPERPLUS_ZOOKEEPER_H

#include <string>

using namespace std;

class zookeeper_plus {
public:
    static zookeeper_plus* newClient(string address, bool retry);


};

#endif //ZOOKEEPERPLUS_ZOOKEEPER_H
