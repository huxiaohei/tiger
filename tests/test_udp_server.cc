/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/12/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/iomanager.h"
#include "../src/macro.h"
#include "../src/socket.h"

void run() {
    auto addr = tiger::Address::LookupAny("0.0.0.0:8085");
    auto socket = tiger::Socket::CreateUDPSocket();
    if (socket->bind(addr)) {
        TIGER_LOG_D(tiger::TEST_LOG) << "[udp_server bind address success [" << addr << "]";
    } else {
        TIGER_LOG_E(tiger::TEST_LOG) << "[udp_server bind address fail [" << addr << "]";
    }
    while (true) {
        char buffer[1024];
        auto from = std::make_shared<tiger::IPv4Address>();
        auto len = socket->recv_from(buffer, 1024, from);
        if (len > 0) {
            buffer[len] = '\0';
            TIGER_LOG_D(tiger::TEST_LOG) << "[udp_server recv '" << buffer << "' from " << *from << "]";
            len = socket->send_to(buffer, len, from);
            if (len < 0) {
                TIGER_LOG_D(tiger::TEST_LOG) << "[upd_server send '" << buffer << "' to " << *from << "]";
            }
        }
    }
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("UDPServer");
    TIGER_LOG_D(tiger::TEST_LOG) << "[udp_server test start]";
    auto iom = std::make_shared<tiger::IOManager>("UDPServer", true, 1);
    iom->schedule(run);
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "[udp_server test end]";
    return 0;
}