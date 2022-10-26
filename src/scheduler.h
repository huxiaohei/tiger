/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_SCHEDULER_H__
#define __TIGER_SCHEDULER_H__

#include <atomic>

#include "macro.h"
#include "mutex.h"

namespace tiger {

class Scheduler : public std::enable_shared_from_this<Scheduler> {
   private:
    struct Task {
        Coroutine::ptr m_co;
        pid_t m_thread_id;

        Task()
            : m_co(nullptr), m_thread_id(0) {
        }

        Task(Coroutine::ptr co, pid_t thread_id = 0)
            : m_co(co), m_thread_id(thread_id) {
        }

        Task(Coroutine::ptr *co, pid_t thread_id = 0)
            : m_co(nullptr), m_thread_id(thread_id) {
            m_co.swap(*co);
        }

        Task(std::function<void()> fn, pid_t thread_id = 0)
            : m_thread_id(thread_id) {
            m_co = std::make_shared<Coroutine>(fn);
        }

        Task(std::function<void()> *fn, pid_t thread_id = 0)
            : m_thread_id(thread_id) {
            m_co = std::make_shared<Coroutine>(*fn, thread_id);
        }

        void reset() {
            m_thread_id = 0;
            m_co = nullptr;
        }
    };

   private:
    std::string m_name;
    bool m_use_main_thread;
    std::atomic<size_t> m_thread_cnt;
    pid_t m_main_thread_id;
    bool m_is_stopping;
    bool m_is_stopped;
    std::atomic<size_t> m_idle_cnt{0};
    std::vector<Thread::ptr> m_threads;
    Semaphore m_semaphore;

   protected:
    std::list<Task> m_tasks;
    MutexLock m_mutex;

   protected:
    virtual void open_hook(){};
    virtual void close_hook(){};
    virtual void tickle();
    virtual void run();
    virtual void idle();

   public:
    typedef std::shared_ptr<Scheduler> ptr;

    Scheduler(const std::string &name = "", bool use_main_thread = false, size_t thread_cnt = 1);
    ~Scheduler();

    const std::string &name() const { return m_name; }
    bool use_main_thread() const { return m_use_main_thread; }
    size_t thread_cnt() const { return m_thread_cnt; }
    pid_t main_thread_id() const { return m_main_thread_id; }
    bool is_stopping() const { return m_is_stopping; }
    bool is_stopped() const { return m_is_stopped; }
    bool has_idel_thread() const { return m_idle_cnt > 0; }

   public:
    static Scheduler::ptr GetThreadScheduler();

   public:
    virtual void start();
    virtual void stop();

   public:
    template <typename T>
    bool schedule(T v, pid_t thread_id = 0) {
        if (m_is_stopping) return false;
        bool need_tickle = false;
        {
            MutexLock::Lock lock(m_mutex);
            need_tickle = m_tasks.empty();
            m_tasks.push_back(Task(v, thread_id));
        }
        if (need_tickle && m_idle_cnt > 0) {
            tickle();
        }
        return true;
    }

    template <typename ForwardIterator>
    bool schedules(ForwardIterator begin, ForwardIterator end, pid_t thread_id = 0) {
        if (m_is_stopping) return false;
        bool need_tickle = false;
        {
            MutexLock::Lock lock(m_mutex);
            need_tickle = m_tasks.empty();
            while (begin != end) {
                m_tasks.push_back(Task(*begin, thread_id));
                ++begin;
            }
        }
        if (need_tickle && m_idle_cnt > 0) {
            tickle();
        }
        return true;
    }
};

}  // namespace tiger

#endif