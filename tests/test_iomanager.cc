/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/hook.h"
#include "../src/iomanager.h"
#include "../src/thread.h"

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

    iom->add_timer(
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

    auto loop_timer = iom->add_timer(
        3000, [iom]() {
            TIGER_LOG_D(tiger::TEST_LOG) << "loop_timer sleep 3000 callback ";
            iom->stop();
        },
        true);

    iom->start();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("IOMANAGER");
    TIGER_LOG_D(tiger::TEST_LOG) << "[http_iomanager test start]";
    test_sleep();
    test_timer();
    TIGER_LOG_D(tiger::TEST_LOG) << "[http_iomanager test end]";
}