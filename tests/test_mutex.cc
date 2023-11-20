/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/29
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include <unistd.h>

#include "../src/macro.h"
#include "../src/mutex.h"

void multi_threads_exec(size_t count, const std::string &name, std::function<void()> callback)
{
    std::vector<tiger::Thread::ptr> v;
    for (size_t i = 0; i < count; ++i)
    {
        v.push_back(std::make_shared<tiger::Thread>(callback, name + std::to_string(i)));
    }
    for (size_t i = 0; i < count; ++i)
    {
        v[i]->join();
    }
}

void test_no_lock()
{
    int suc = 0;
    multi_threads_exec(10, "no_lock", [&suc]() {
        for (size_t i = 0; i < 1000; ++i)
        {
            usleep(2);
            ++suc;
        }
    });
    if (suc <= 10000)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_no_lock suc:" << suc;
    }
    else
    {
        TIGER_LOG_E(tiger::TEST_LOG) << "test_no_lock fail:" << suc;
        throw std::runtime_error("test_no_lock error");
    }
}

void test_spin_lock()
{
    int suc = 0;
    tiger::SpinLock spin_lock;
    multi_threads_exec(10, "spin_lock", [&suc, &spin_lock]() {
        for (size_t i = 0; i < 1000; ++i)
        {
            tiger::SpinLock::Lock lock(spin_lock);
            usleep(2);
            ++suc;
        }
    });
    if (suc == 10000)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_spin_lock suc:" << suc;
    }
    else
    {
        TIGER_LOG_E(tiger::TEST_LOG) << "test_spin_lock fail:" << suc;
        throw std::runtime_error("test_spin_lock error");
    }
}

void test_mutex_lock()
{
    int suc = 0;
    tiger::MutexLock mutex_lock;
    multi_threads_exec(10, "mutex_lock", [&suc, &mutex_lock]() {
        for (size_t i = 0; i < 1000; ++i)
        {
            tiger::MutexLock::Lock lock(mutex_lock);
            usleep(2);
            ++suc;
        }
    });
    if (suc == 10000)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_mutex_lock suc:" << suc;
    }
    else
    {
        TIGER_LOG_E(tiger::TEST_LOG) << "test_mutex_lock fail:" << suc;
        throw std::runtime_error("test_mutex_lock error");
    }
}

void test_read_lock()
{
    int suc = 0;
    tiger::ReadWriteLock rd_lock;
    multi_threads_exec(10, "read_lock", [&suc, &rd_lock]() {
        for (size_t i = 0; i < 1000; ++i)
        {
            tiger::ReadWriteLock::ReadLock lock(rd_lock);
            usleep(2);
            ++suc;
        }
    });
    if (suc <= 10000)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_read_lock suc:" << suc;
    }
    else
    {
        TIGER_LOG_E(tiger::TEST_LOG) << "test_read_lock fail:" << suc;
        throw std::logic_error("test_read_lock error");
    }
}

void test_write_lock()
{
    int suc = 0;
    tiger::ReadWriteLock wr_lock;
    multi_threads_exec(10, "write_lock", [&suc, &wr_lock]() {
        for (size_t i = 0; i < 1000; ++i)
        {
            tiger::ReadWriteLock::WriteLock lock(wr_lock);
            usleep(2);
            ++suc;
        }
    });
    if (suc == 10000)
    {
        TIGER_LOG_D(tiger::TEST_LOG) << "test_write_lock suc:" << suc;
    }
    else
    {
        TIGER_LOG_E(tiger::TEST_LOG) << "test_write_lock fail:" << suc;
        throw std::runtime_error("test_write_lock error");
    }
}

int main()
{
    tiger::SingletonLoggerMgr::Instance()->add_loggers("log", "../conf/tiger.yml");
    tiger::Thread::SetName("MUTEX");
    TIGER_LOG_D(tiger::TEST_LOG) << "[test mutex start]";
    test_no_lock();
    test_spin_lock();
    test_mutex_lock();
    test_read_lock();
    test_write_lock();
    TIGER_LOG_D(tiger::TEST_LOG) << "[test mutex end]";
    return 0;
}