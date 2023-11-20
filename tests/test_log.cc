/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/macro.h"
#include "../src/thread.h"

void test_log_performance()
{
    auto t = tiger::Millisecond();
    int a = 100;
    for (auto i = 0; i < 100000; ++i)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "TEST_LOG_FILE_DEBUG";
        TIGER_LOG_I(tiger::TEST_LOG) << "TEST_LOG_FILE_INFO";
        TIGER_LOG_W(tiger::TEST_LOG) << "TEST_LOG_FILE_WARN";
        TIGER_LOG_E(tiger::TEST_LOG) << "TEST_LOG_FILE_ERROR";

        TIGER_LOG_FMT_D(tiger::TEST_LOG, "%s %d %p", "TIGER_LOG_FMT_D", a, &a);
        TIGER_LOG_FMT_I(tiger::TEST_LOG, "%s %d %p", "TIGER_LOG_FMT_I", a, &a);
        TIGER_LOG_FMT_W(tiger::TEST_LOG, "%s %d %p", "TIGER_LOG_FMT_W", a, &a);
        TIGER_LOG_FMT_E(tiger::TEST_LOG, "%s %d %p", "TIGER_LOG_FMT_E", a, &a);
    }
    TIGER_LOG_D(tiger::TEST_LOG) << "[performance test cost: " << tiger::Millisecond() - t << "ms]";
}

void test_log_thread()
{
    std::vector<tiger::Thread::ptr> v;
    for (size_t i = 0; i < 3; ++i)
    {
        v.push_back(std::make_shared<tiger::Thread>(
            []() {
                for (size_t i = 0; i < 100000; ++i)
                {
                    TIGER_LOG_D(tiger::TEST_LOG) << "LOG " << i;
                }
            },
            "LogThread"));
    }
    for (size_t i = 0; i < v.size(); ++i)
    {
        v[i]->join();
    }
}

int main()
{
    tiger::SingletonLoggerMgr::Instance()->add_loggers("log", "../conf/tiger.yml");
    tiger::Thread::SetName("LOG");
    TIGER_LOG_D(tiger::TEST_LOG) << "[test log start]";
    test_log_thread();
    test_log_performance();
    TIGER_LOG_D(tiger::TEST_LOG) << "[test log end]";
    return 0;
}
