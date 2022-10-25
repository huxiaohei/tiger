/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/servers/http/http_connection.h"

void test_http_result() {
    auto rsp = std::make_shared<tiger::http::HTTPResponse>();
    rsp->set_close(true);
    rsp->set_header("Host", "http://www.huxiaohei.com");
    rsp->set_body("Hello! I'm tigerkin!");
    auto ret = std::make_shared<tiger::http::HTTPResult>(rsp,
                                                         tiger::http::HTTPResult::ErrorCode::OK,
                                                         "ok");
    TIGER_LOG_D(tiger::TEST_LOG) << ret->to_string();
}

void test_http_connection() {
    auto iom = std::make_shared<tiger::IOManager>("HTTP", true);
    iom->schedule([iom]() {
        auto rst = tiger::http::HTTPConnection::Get("http://www.baidu.com", 5000);
        TIGER_LOG_D(tiger::TEST_LOG) << rst->to_string();
        iom->stop();
    });
    iom->start();
}

void test_http_connection_pool() {
    auto iom = std::make_shared<tiger::IOManager>("HTTP_POOL", true, 1);
    auto conn_pool = tiger::http::HTTPConnectionPool::Create("http://www.huxiaohei.com", "", 20, 30000, 5);
    static std::atomic<int> i{0};
    iom->add_timer(
        1000, [iom, conn_pool]() {
            ++i;
            if (i == 20) {
                iom->schedule([iom]() {
                    iom->stop();
                });
            }
            TIGER_LOG_D(tiger::TEST_LOG) << "get start " << i;
            auto rst = conn_pool->get("/ChapterOne/", 5000);
            TIGER_LOG_D(tiger::TEST_LOG) << "get end";
        },
        true);
    iom->start();
}

int main(int argc, char **argv) {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    // test_http_result();
    // test_http_connection();
    test_http_connection_pool();
    return 0;
}