/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "scheduler.h"

namespace tiger {

static thread_local Scheduler::ptr s_t_scheduler = nullptr;

Scheduler::ptr Scheduler::GetThreadScheduler() {
    return s_t_scheduler;
}

Scheduler::Scheduler(const std::string &name, bool use_main_thread, size_t thread_cnt)
    : m_name(name), m_use_main_thread(use_main_thread), m_thread_cnt(thread_cnt) {
    m_main_thread_id = Thread::CurThreadId();
    m_is_stopping = false;
    m_is_stopped = true;
    if (m_use_main_thread) {
        --m_thread_cnt;
    }
}

Scheduler::~Scheduler() {
    m_is_stopped = true;
    m_is_stopping = false;
    s_t_scheduler = nullptr;
}

void Scheduler::start() {
    {
        MutexLock::Lock lock(m_mutex);
        if (!m_is_stopped || m_is_stopping) {
            return;
        }
        m_is_stopped = false;
        m_is_stopping = false;
        for (size_t i = 0; i < m_thread_cnt; ++i) {
            m_threads.push_back(std::make_shared<Thread>(std::bind(&Scheduler::run, this), m_name + std::to_string(i + 1)));
        }
    }
    if (m_use_main_thread) {
        run();
    }
}

void Scheduler::stop() {
    if (m_is_stopped) return;
    m_is_stopping = true;
    tickle();
}

void Scheduler::tickle() {
    if (m_idle_cnt) {
        m_semaphore.post();
    }
}

void Scheduler::run() {
    open_hook();
    auto idle_co = std::make_shared<Coroutine>(std::bind(&Scheduler::idle, this));
    s_t_scheduler = shared_from_this();
    Task task;
    while (true) {
        task.reset();
        bool is_active = false;
        bool need_tickle = false;
        {
            MutexLock::Lock lock(m_mutex);
            auto t = m_tasks.begin();
            while (t != m_tasks.end()) {
                if (t->m_thread_id != 0 && t->m_thread_id != Thread::CurThreadId()) {
                    ++t;
                    continue;
                }
                task = *t;
                m_tasks.erase(t);
                if (!task.m_co) {
                    task.reset();
                    continue;
                }
                is_active = true;
                break;
            }
            if (!m_tasks.empty() && m_idle_cnt > 0) {
                need_tickle = true;
            }
        }
        if (need_tickle) {
            tickle();
        }
        if (is_active) {
            if (task.m_co->state() & (Coroutine::State::INIT | Coroutine::State::YIELD)) {
                task.m_co->resume();
            } else {
                TIGER_LOG_E(SYSTEM_LOG) << "[the coroutine is nullptr]";
            }
        } else {
            if (m_is_stopping) {
                if (Thread::CurThreadId() == m_main_thread_id && m_thread_cnt != 0) {
                    idle_co->resume();
                    continue;
                }
                break;
            } else {
                idle_co->resume();
            }
        }
    }
    --m_thread_cnt;
    tickle();
}

void Scheduler::idle() {
    TIGER_LOG_D(TEST_LOG) << "[enter idle]";
    while (true) {
        if (m_is_stopping) {
            if (m_main_thread_id == Thread::CurThreadId() && m_thread_cnt > 0) {
                ++m_idle_cnt;
                m_semaphore.wait();
                --m_idle_cnt;
                Coroutine::Yield();
                continue;
            }
            break;
        } else {
            ++m_idle_cnt;
            m_semaphore.wait();
            --m_idle_cnt;
            Coroutine::Yield();
        }
    }
    TIGER_LOG_D(TEST_LOG) << "[idle exit]";
}

}  // namespace tiger