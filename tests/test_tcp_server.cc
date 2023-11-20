/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

class EchoServer : public tiger::TCPServer
{
  public:
    void handle_client(tiger::Socket::ptr client) override
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "[handle client:" << client << "]";
        auto buf = std::make_shared<tiger::ByteArray>();
        while (true)
        {
            buf->clear();
            std::vector<iovec> iovs;
            buf->get_enable_write_buffers(iovs, 1024);
            int rt = client->recv(&iovs[0], iovs.size());
            if (rt == 0)
            {
                TIGER_LOG_I(tiger::TEST_LOG) << "[has closed client:" << client << "]";
                break;
            }
            else if (rt < 0)
            {
                TIGER_LOG_E(tiger::TEST_LOG) << "[client error"
                                             << " erron:" << strerror(errno) << "]";
                break;
            }
            buf->set_position(buf->get_position() + rt);
            buf->set_position(0);
            const std::string &msg = buf->to_string();
            TIGER_LOG_D(tiger::TEST_LOG) << "[Echo receive: " << msg << "]";
            client->send(msg.c_str(), msg.size());
            if (msg.find("stop") == 0)
            {
                TIGER_LOG_I(tiger::TEST_LOG) << "[ECHO STOP]";
                stop();
                tiger::IOManager::GetThreadIOM()->stop();
            }
        }
    }
};

void run()
{
    auto addr = tiger::Address::LookupAny("0.0.0.0:8080");
    auto ech_server = std::make_shared<EchoServer>();
    ech_server->bind(addr);
    ech_server->start();
}

int main()
{
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("TCPServer");
    TIGER_LOG_D(tiger::TEST_LOG) << "[tcp_server test start]";
    auto iom = std::make_shared<tiger::IOManager>("TCPServer", true, 1);
    iom->schedule(run);
    iom->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "[tcp_server test end]";
    return 0;
}