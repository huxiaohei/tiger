/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/servers/http/http_parser.h"

void test_parser_request() {
    const char request_data[] =
        "POST / HTTP/1.1\r\n"
        "Host: www.huxiaohei.com\r\n"
        "Content-Length: 11\r\n\r\n"
        "hello world";

    tiger::http::HTTPRequestParser parser;
    std::string tmp = request_data;
    size_t rt = parser.execute(&tmp[0], tmp.size());
    TIGER_LOG_D(tiger::TEST_LOG) << "[rt:" << rt
                                 << " finished:" << parser.is_finished()
                                 << " error:" << parser.has_error()
                                 << " content-length:" << parser.get_content_length() << "]";
    tmp.resize(tmp.size() - rt);
    TIGER_LOG_D(tiger::TEST_LOG) << "[\n"
                                 << parser.get_data()->to_string() << "\n]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[\n"
                                 << tmp << "\n]";
}

void test_parser_response() {
    const char response_data[] =
        "HTTP/1.1 405 Not Allowed\r\n"
        "Server: nginx/1.20.1\r\n"
        "Date: Sun, 28 Nov 2021 10:49:35 GMT\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 157\r\n"
        "Connection: close\r\n\r\n"
        "<html>\r\n"
        "<head><title>405 Not Allowed</title></head>\r\n"
        "<body>\r\n"
        "<center><h1>405 Not Allowed</h1></center>\r\n"
        "<hr><center>nginx/1.20.1</center>\r\n"
        "</body>\r\n"
        "</html>";
    tiger::http::HTTPResponseParser parser;
    std::string tmp = response_data;
    size_t rt = parser.execute(&tmp[0], tmp.size(), false);
    TIGER_LOG_D(tiger::TEST_LOG) << "[rt:" << rt
                                 << " finished:" << parser.is_finished()
                                 << " error:" << parser.has_error()
                                 << " content-length:" << parser.get_content_length() << "]";
    tmp.resize(tmp.size() - rt);
    TIGER_LOG_D(tiger::TEST_LOG) << "[\n"
                                 << parser.get_data()->to_string() << "\n]";
    TIGER_LOG_D(tiger::TEST_LOG) << "[\n"
                                 << tmp << "\n]";
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("HTTP_PARSER");
    TIGER_LOG_D(tiger::TEST_LOG) << "[http_parser test start]";
    test_parser_request();
    test_parser_response();
    TIGER_LOG_D(tiger::TEST_LOG) << "[http_parser test end]";
}