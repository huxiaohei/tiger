/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void func_1() {
    TIGER_LOG_D(tiger::TEST_LOG) << "func_1 start";
    usleep_f(100);
    TIGER_LOG_D(tiger::TEST_LOG) << "func_1 end";
}

void func_2() {
    TIGER_LOG_D(tiger::TEST_LOG) << "func_2 start";
    usleep_f(200);
    TIGER_LOG_D(tiger::TEST_LOG) << "func_2 end";
}

void test_single_thread_scheduler() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test_single_thread_scheduler start";
    auto scheduler = std::make_shared<tiger::Scheduler>("SingleThread", true);
    scheduler->schedule(&func_1);
    scheduler->schedule([scheduler]() {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_single_thread_scheduler stop";
        scheduler->stop();
    });
    scheduler->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "test_single_thread_scheduler end";
}

void test_multi_thread_with_main_thread() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test_multi_thread_with_main_thread start";
    tiger::Thread::SetName("MultiThread0");
    auto scheduler = std::make_shared<tiger::Scheduler>("MultiThread", true, 4);
    for (size_t i = 0; i < 20; ++i) {
        scheduler->schedule(&func_1);
    }
    scheduler->schedule([scheduler]() {
        sleep(2);
        for (size_t i = 0; i < 10; ++i) {
            scheduler->schedule(&func_2);
        }
        scheduler->stop();
    });
    scheduler->start();
    TIGER_LOG_D(tiger::TEST_LOG) << "test_multi_thread_with_main_thread end";
}

int main() {
    tiger::Thread::SetName("Main");
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    TIGER_LOG_D(tiger::TEST_LOG) << "test_scheduler start";
    test_multi_thread_with_main_thread();
    tiger::Thread::SetName("Main");
    TIGER_LOG_D(tiger::TEST_LOG) << "test_scheduler end";
}