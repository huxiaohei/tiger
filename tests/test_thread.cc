/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/hook.h"
#include "../src/macro.h"
#include "../src/thread.h"

void thread_callback() {
    TIGER_LOG_D(tiger::TEST_LOG) << "thread_callback start";
    sleep(30);
    TIGER_LOG_D(tiger::TEST_LOG) << "thread_callback end";
}

void test_create_threads(size_t count) {
    std::vector<tiger::Thread::ptr> v;
    for (size_t i = 0; i < count; ++i) {
        v.push_back(std::make_shared<tiger::Thread>(&thread_callback, "Thread_" + std::to_string(i)));
    }
    for (size_t i = 0; i < v.size(); ++i) {
        v[i]->join();
    }
}

void test_thread_cond() {
    std::vector<tiger::Thread::ptr> v;

    pthread_cond_t cond;
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);
    for (size_t i = 0; i < 4; ++i) {
        v.push_back(
            std::make_shared<tiger::Thread>([i, &cond, &mutex]() {
                if (i % 2 == 0) {
                    TIGER_LOG_D(tiger::TEST_LOG) << "Cond start wait";
                    tiger::Thread::CondWait(cond, mutex);
                    TIGER_LOG_D(tiger::TEST_LOG) << "Cond end wait";
                } else {
                    TIGER_LOG_D(tiger::TEST_LOG) << "Cond start sleep";
                    sleep(30);
                    TIGER_LOG_D(tiger::TEST_LOG) << "Cond signal";
                    tiger::Thread::CondSignal(cond, mutex);
                }
            },
                                            "Cond_" + std::to_string(i)));
    }
    for (size_t i = 0; i < 4; ++i) {
        v[i]->join();
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

int main() {
    tiger::SingletonLoggerMgr::Instance()->add_loggers("tiger", "../conf/tiger.yml");
    tiger::Thread::SetName("THREAD");
    TIGER_LOG_D(tiger::TEST_LOG) << "[test thread start]";
    test_create_threads(10);
    test_thread_cond();
    TIGER_LOG_D(tiger::TEST_LOG) << "[test thread end]";
    return 0;
}