# tiger

## 开发环境

* CentOS Stream release 9
* ragel version 6.10
* cmake version 3.20.2
* yaml-cpp version 0.6.2
* g++ version 11.3.1
* OpenSSL version 3.0.1
* fmt version 9.1.0

## 说明文档

<https://blog.csdn.net/huxiaoheigame/category_12079132.html?spm=1001.2014.3001.5482>

## 示例

### HTTP服务

```cpp

#include "tiger.h"

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
```

### TCP

```cpp

#include "tiger.h"

class EchoServer : public tiger::TCPServer {
   public:
    void handle_client(tiger::Socket::ptr client) override {
        TIGER_LOG_D(tiger::TEST_LOG) << "[handle client:" << client << "]";
        auto buf = std::make_shared<tiger::ByteArray>();
        while (true) {
            buf->clear();
            std::vector<iovec> iovs;
            buf->get_enable_write_buffers(iovs, 1024);
            int rt = client->recv(&iovs[0], iovs.size());
            if (rt == 0) {
                TIGER_LOG_I(tiger::TEST_LOG) << "[has closed client:" << client << "]";
                break;
            } else if (rt < 0) {
                TIGER_LOG_E(tiger::TEST_LOG) << "[client error"
                                             << " erron:" << strerror(errno) << "]";
                break;
            }
            buf->set_position(buf->get_position() + rt);
            buf->set_position(0);
            const std::string &msg = buf->to_string();
            TIGER_LOG_D(tiger::TEST_LOG) << "[Echo receive: " << msg << "]";
            client->send(msg.c_str(), msg.size());
            if (msg.find("stop") == 0) {
                TIGER_LOG_I(tiger::TEST_LOG) << "[ECHO STOP]";
                stop();
                tiger::IOManager::GetThreadIOM()->stop();
            }
        }
    }
};

void run() {
    auto addr = tiger::Address::LookupAny("0.0.0.0:8080");
    auto ech_server = std::make_shared<EchoServer>();
    ech_server->bind(addr);
    ech_server->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("TCPServer");
    TIGER_LOG_D(tiger::TEST_LOG) << "[tcp_server test start]";
    auto iom = std::make_shared<tiger::IOManager>("TCPServer", true, 1);
    iom->schedule(run);
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "[tcp_server test end]";
    return 0;
}
```
