/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/02
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/coroutine.h"
#include "../src/macro.h"
#include "../src/thread.h"

void test_fn_1() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_1 resume";
}

void test_fn_2() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_2 resume";
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_2 yiled";
    tiger::Coroutine::Yield();
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_2 resume";
}

void test_fn_3() {
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_3 resume";
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_3 yield";
    tiger::Coroutine::Yield();
    TIGER_LOG_D(tiger::TEST_LOG) << "test_fn_3 resume";
}

void test_coroutine() {
    auto co_1 = std::make_shared<tiger::Coroutine>(&test_fn_1);
    auto co_2 = std::make_shared<tiger::Coroutine>(&test_fn_2);
    co_1->resume();
    co_2->resume();
    co_2->resume();
    co_2->reset(&test_fn_3);
    co_2->resume();
    co_2->resume();
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("COROUTINE");
    TIGER_LOG_D(tiger::TEST_LOG) << "[coroutine test start]";
    test_coroutine();
    TIGER_LOG_D(tiger::TEST_LOG) << "[coroutine test end]";
    return 0;
}