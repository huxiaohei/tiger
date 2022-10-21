/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "thread.h"

#include <sys/syscall.h>
#include <unistd.h>

#include "const.h"
#include "macro.h"

namespace tiger {

static thread_local Thread *cur_thread = nullptr;
static thread_local std::string cur_thread_name = "UNKNOW";
static thread_local pid_t cur_thread_id = 0;

Thread::Thread(std::function<void()> cb, const std::string &name)
    : m_cb(cb), m_name(name) {
    if (name.empty()) {
        m_name = "UNKNOW";
    }
    auto it = pthread_create(&m_thread, nullptr, &Thread::Run, this);
    if (it) {
        TIGER_LOG_E(SYSTEM_LOG) << "[pthread_create fail]";
        throw std::logic_error("[pthread_create fail]");
    }
    m_semaphore.wait();
}

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if (m_thread) {
        auto it = pthread_join(m_thread, nullptr);
        if (it) {
            TIGER_LOG_E(SYSTEM_LOG) << "[pthread_join fail]";
            throw std::logic_error("[pthread_join fail]");
        }
    }
    m_thread = 0;
}

void Thread::SetName(const std::string &name) {
    cur_thread_name = name;
    if (cur_thread) {
        cur_thread->m_name = name;
        pthread_setname_np(cur_thread->m_thread, cur_thread->m_name.substr(0, 15).c_str());
    }
}

const std::string &Thread::CurThreadName() {
    return cur_thread_name;
}

const pid_t Thread::CurThreadId() {
    if (cur_thread_id == 0) {
        cur_thread_id = syscall(SYS_gettid);
    }
    return cur_thread_id;
}

void Thread::CondWait(pthread_cond_t &cond, pthread_mutex_t &mtx) {
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&cond, &mtx);
    pthread_mutex_unlock(&mtx);
}

void Thread::CondSignal(pthread_cond_t &cond, pthread_mutex_t &mtx, size_t cnt) {
    pthread_mutex_lock(&mtx);
    while (cnt != 0) {
        pthread_cond_signal(&cond);
        --cnt;
    }
    pthread_mutex_unlock(&mtx);
}

void *Thread::Run(void *arg) {
    try {
        Thread *thread = (Thread *)arg;
        cur_thread = thread;
        cur_thread_name = thread->m_name;
        cur_thread_id = syscall(SYS_gettid);
        pthread_setname_np(thread->m_thread, thread->m_name.substr(0, 15).c_str());
        std::function<void()> cb;
        cb.swap(thread->m_cb);
        thread->m_semaphore.post();
        cb();
    } catch (const std::exception &e) {
        TIGER_LOG_E(SYSTEM_LOG) << "[thread run fail " << e.what() << "]";
    }
    return 0;
}

}  // namespace tiger