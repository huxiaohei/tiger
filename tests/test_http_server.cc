/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/26
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

class Hello : public tiger::http::Servlet {
   public:
    int32_t handle(tiger::http::HTTPRequest::ptr request,
                   tiger::http::HTTPResponse::ptr response,
                   tiger::http::HTTPSession::ptr session) override {
        response->set_body("Hello! I'm tiger!");
        return 0;
    }
};

class Bye : public tiger::http::Servlet {
   public:
    int32_t handle(tiger::http::HTTPRequest::ptr request,
                   tiger::http::HTTPResponse::ptr response,
                   tiger::http::HTTPSession::ptr session) override {
        response->set_body("Bye!");
        return 0;
    }
};

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("HTTP_SERVER");
    TIGER_LOG_D(tiger::TEST_LOG) << "[http_server test start]";

    auto iom = std::make_shared<tiger::IOManager>("HTTP_SERVER", true, 1);
    iom->schedule([]() {
        auto server = std::make_shared<tiger::http::HTTPServer>();
        auto addr = tiger::IPAddress::LookupAny("0.0.0.0:8080");
        auto ssl_addr = tiger::IPAddress::LookupAny("0.0.0.0:8081");
        server->bind(addr, false);
        server->bind(ssl_addr, true);
        auto dsp = server->get_servlet_dispatch();
        dsp->add_servlet("/hello", std::make_shared<Hello>());
        dsp->add_servlet("/bye", std::make_shared<Bye>());
        dsp->add_servlet(
            "/close", [server](tiger::http::HTTPRequest::ptr request,
                               tiger::http::HTTPResponse::ptr response,
                               tiger::http::HTTPSession::ptr session) {
                server->stop();
                tiger::IOManager::GetThreadIOM()->stop();
                return 0;
            });
        server->load_certificates("./tiger.crt", "./tiger.key");
        server->start();
    });
    iom->start();

    TIGER_LOG_D(tiger::TEST_LOG) << "[http_server test end]";
}