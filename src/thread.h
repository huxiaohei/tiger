/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_THREAD_H__
#define __TIGER_THREAD_H__

#include <pthread.h>

#include <functional>
#include <memory>
#include <string>

#include "mutex.h"

namespace tiger
{

class Thread
{
  private:
    pthread_t m_thread;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;

  public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();

    const std::string &name() const
    {
        return m_name;
    }

    void join();

  public:
    static void SetName(const std::string &name);
    static const std::string &CurThreadName();
    static const pid_t CurThreadId();
    static void CondWait(pthread_cond_t &cond, pthread_mutex_t &mtx);
    static void CondSignal(pthread_cond_t &cond, pthread_mutex_t &mtx, size_t cnt = 1);

  private:
    static void *Run(void *arg);

  private:
    Thread(const Thread &) = delete;
    Thread(const Thread &&) = delete;
    Thread &operator=(const Thread &) = delete;
};

} // namespace tiger

#endif