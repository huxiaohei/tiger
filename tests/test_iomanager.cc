/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void sleep_func1() {
    TIGER_LOG_D(tiger::TEST_LOG) << "sleep 1000ms start";
    usleep(1000);
    TIGER_LOG_D(tiger::TEST_LOG) << "sleep 1000ms end";
}

void sleep_func2() {
    TIGER_LOG_D(tiger::TEST_LOG) << "sleep 5000ms start";
    usleep(5000);
    TIGER_LOG_D(tiger::TEST_LOG) << "sleep 5000ms end";
    auto iom = tiger::IOManager::GetThreadIOM();
    iom->stop();
}

void test_sleep() {
    auto iom = std::make_shared<tiger::IOManager>("IOManager", true, 1);
    iom->schedule(sleep_func1);
    iom->schedule(sleep_func2);
    iom->start();
}

void test_timer() {
    auto iom = std::make_shared<tiger::IOManager>("IOManager", true, 5);
    tiger::TimerManager::Timer::ptr can_loop_timer = iom->add_timer(
        500, []() {
            TIGER_LOG_D(tiger::TEST_LOG) << "loop_timer sleep 500 callback ";
        },
        true);
    for (int i = 0; i < 100; ++i) {
        time_t ms = i * 10;
        iom->add_timer(
            ms, [iom, ms]() {
                TIGER_LOG_D(tiger::TEST_LOG) << "sleep " << ms << " callback";
            },
            false);
    }
    tiger::TimerManager::Timer::ptr loop_timer = iom->add_timer(
        3000, [can_loop_timer, iom]() {
            TIGER_LOG_D(tiger::TEST_LOG) << "loop_timer sleep 3000 callback ";
            if (iom->is_valid_timer(can_loop_timer))
                iom->cancel_timer(can_loop_timer);
            else
                iom->stop();
        },
        true);
    tiger::Thread::SetName("IOManager0");
    iom->start();
}

void test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "14.215.177.39", &addr.sin_addr.s_addr);
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    if (rt) {
        TIGER_LOG_D(tiger::TEST_LOG) << "socket connect error"
                                     << "\n\trt:" << rt
                                     << "\n\tstrerror:" << strerror(errno);
        return;
    }
    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    if (rt <= 0) {
        TIGER_LOG_D(tiger::TEST_LOG) << "socket send error"
                                     << "\n\trt:" << rt
                                     << "\n\tstrerror:" << strerror(errno);
        return;
    }
    std::string buff;
    buff.resize(4096);
    rt = recv(sock, &buff[0], buff.size(), 0);
    if (rt <= 0) {
        buff.resize(0);
        TIGER_LOG_D(tiger::TEST_LOG) << "socket recv error"
                                     << "\n\trt:" << rt
                                     << "\n\tstrerror:" << strerror(errno);
        return;
    }
    buff.resize(rt);
    TIGER_LOG_D(tiger::TEST_LOG) << "socket recv msg:\n"
                                 << buff;
    auto iom = tiger::IOManager::GetThreadIOM();
    iom->stop();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    test_sleep();
    test_timer();
    auto iom = std::make_shared<tiger::IOManager>("IOManager", true, 1);
    iom->schedule(test_socket);
    iom->start();
}