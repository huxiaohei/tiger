/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/servers/http/ws_connection.h"

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("WS_CLIENT");
    TIGER_LOG_D(tiger::TEST_LOG) << "[ws_client test start]";
    auto rt = tiger::http::WSConnection::Create("ws://127.0.0.1:8080/echo");
    if (!rt.second) {
        TIGER_LOG_D(tiger::TEST_LOG) << "[==========" << rt.first->to_string() << "]";
        return -1;
    }
    auto iom = std::make_shared<tiger::IOManager>("WS", true, 1);
    int i = 0;
    iom->add_timer(
        3000, [rt, &i]() {
            std::stringstream ss;
            ss << "Hello " << i;
            rt.second->send_message(ss.str(), tiger::http::WSFrameHead::TEXT_FRAME);
            auto msg = rt.second->recv_message();
            if (msg) {
                TIGER_LOG_D(tiger::TEST_LOG) << "[client recv " << msg->get_data() << "]";
            }
            ++i;
        },
        true);
    iom->start();
}