/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/servers/http/ws_server.h"

class WSEchoServlet : public tiger::http::WSServlet
{
    int32_t on_connect(tiger::http::HTTPRequest::ptr header, tiger::http::WSSession::ptr session)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "[websocket connect " << session->get_remote_address() << "]";
        return 0;
    }

    int32_t on_close(tiger::http::HTTPRequest::ptr header, tiger::http::WSSession::ptr session)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "[websocket close " << session->get_remote_address() << "]";
        return 0;
    }

    int32_t handle(tiger::http::HTTPRequest::ptr header, tiger::http::WSFrameMessage::ptr msg,
                   tiger::http::WSSession::ptr session)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "[websocket recv " << msg->get_data() << "]";
        session->send_message(msg->get_data());
        return 0;
    }
};

int main()
{
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("WS_SERVER");
    TIGER_LOG_D(tiger::TEST_LOG) << "[ws_server test start]";
    auto iom = std::make_shared<tiger::IOManager>("WS", true, 1);
    iom->schedule([]() {
        auto ws_server = std::make_shared<tiger::http::WSServer>();
        auto addr = tiger::IPAddress::LookupAny("0.0.0.0:8080");
        ws_server->bind(addr, false);
        ws_server->get_servlet_dispatch()->add_servlet("/echo", std::make_shared<WSEchoServlet>());
        ws_server->start();
    });
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "[ws_server test end]";
}