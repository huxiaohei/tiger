/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/22
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void test_socket_send(const std::string &host, tiger::IOManager::ptr iom) {
    tiger::IPAddress::ptr addr = tiger::IPAddress::LookupAny(host);
    tiger::Socket::ptr socket = tiger::Socket::CreateTCPSocket();
    if (!socket->connect(addr)) {
        TIGER_LOG_E(tiger::TEST_LOG) << "[socket connnect fail errno:" << strerror(errno) << "]";
        return;
    }
    const char buffer[] = "GET / HTTP/1.0\r\n\r\n";
    ssize_t rt = socket->send(buffer, sizeof(buffer));
    if (rt <= 0) {
        TIGER_LOG_E(tiger::TEST_LOG) << "[socket send fail connected:" << socket->is_connected()
                                     << " errno:" << strerror(errno) << "]";
        return;
    }
    std::string buffs;
    buffs.resize(4096);
    rt = socket->recv(&buffs[0], buffs.size());
    if (rt <= 0) {
        TIGER_LOG_E(tiger::TEST_LOG) << "socket recv fail connected:" << socket->is_connected()
                                     << " errno:" << strerror(errno) << "]";
        return;
    }
    buffs.resize(rt);
    TIGER_LOG_D(tiger::TEST_LOG) << "socket recv:" << buffs;
    iom->stop();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    TIGER_LOG_D(tiger::TEST_LOG) << "socket test start";
    tiger::IOManager::ptr iom = std::make_shared<tiger::IOManager>("TEST_SOCKET", true, 1);
    iom->schedule(std::bind(test_socket_send, "www.baidu.com:80", iom));
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "socket test end";
}