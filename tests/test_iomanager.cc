/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void test_timer() {
    auto iom = std::make_shared<tiger::IOManager>("IOManager", true, 5);
    for (int i = 0; i < 100; ++i) {
        iom->add_timer(
            10 * i, [iom]() {
                TIGER_LOG_D(tiger::TEST_LOG) << "sleep 5000 timer callback";
            },
            false);
    }
    tiger::TimerManager::Timer::ptr can_loop_timer = iom->add_timer(
        500, []() {
            TIGER_LOG_D(tiger::TEST_LOG) << "loop_timer sleep 500 callback ";
        },
        true);
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

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    test_timer();
}