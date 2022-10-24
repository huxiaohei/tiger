/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void run() {
    auto addr = tiger::Address::LookupAny("0.0.0.0:8035");
    auto tcp_server = std::make_shared<tiger::TCPServer>();
    tcp_server->bind(addr);
    tcp_server->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    TIGER_LOG_D(tiger::TEST_LOG) << "tcp_server test start";
    auto iom = std::make_shared<tiger::IOManager>("IOM", true, 1);
    iom->schedule(run);
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "tcp_server test start";
    return 0;
}