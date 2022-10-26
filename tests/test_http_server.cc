/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/26
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/servers/http/http_server.h"

void run() {
    auto server = std::make_shared<tiger::http::HTTPServer>();
    auto addr = tiger::IPAddress::LookupAny("0.0.0.0:8080");
    if (!server->bind(addr)) {
        TIGER_LOG_E(tiger::TEST_LOG) << "[bind error "
                                     << addr << "]";
        return;
    }
    auto dsp = server->get_servlet_dispatch();
    dsp->add_servlet("/hello", [](tiger::http::HTTPRequest::ptr req,
                                  tiger::http::HTTPResponse::ptr rsp,
                                  tiger::http::HTTPSession::ptr session) {
        TIGER_LOG_D(tiger::TEST_LOG) << "hello " << req;
        rsp->set_body("Hello I'm tiger!");
        rsp->set_header("result", "ok");
        rsp->set_cookies("cookies", "tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << "hello " << rsp;
        return 0;
    });

    dsp->add_servlet("/bye", [](tiger::http::HTTPRequest::ptr req,
                                tiger::http::HTTPResponse::ptr rsp,
                                tiger::http::HTTPSession::ptr session) {
        TIGER_LOG_D(tiger::TEST_LOG) << "hello " << req;
        rsp->set_body("Bye I'm tiger!");
        rsp->set_header("result", "ok");
        rsp->set_cookies("cookies", "tiger");
        TIGER_LOG_D(tiger::TEST_LOG) << "hello " << rsp;
        tiger::IOManager::GetThreadIOM()->stop();
        return 0;
    });
    server->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    auto iom = std::make_shared<tiger::IOManager>("HTTP_SERVER", true, 2);
    iom->schedule(&run);
    iom->start();
}