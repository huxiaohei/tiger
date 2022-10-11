/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_TIMER_H__
#define __TIGER_TIMER_H__

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "mutex.h"

namespace tiger {

class TimerManager {
   public:
    class Timer {
       private:
        friend TimerManager;

       private:
        size_t m_id;
        time_t m_interval;
        bool m_loop;
        time_t m_next_time;
        std::function<void()> m_cb;

       public:
        typedef std::shared_ptr<Timer> ptr;

        explicit Timer(time_t ms, bool loop, std::function<void()> cb);

       public:
        struct Comparator {
            bool operator()(const Timer::ptr &l, const Timer::ptr &r) const;
        };
    };

   private:
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    ReadWriteLock m_lock;

   protected:
    virtual void on_timer_refresh() = 0;

   public:
    typedef std::shared_ptr<TimerManager> ptr;

    TimerManager();
    virtual ~TimerManager();

   public:
    Timer::ptr add_timer(time_t interval, std::function<void()> cb, bool loop = false);
    Timer::ptr add_timer(time_t interval, std::function<void()> *cb, bool loop = false);
    Timer::ptr add_cond_timer(time_t interval, std::function<void()> cb, std::weak_ptr<void> cond, bool loop = false);
    Timer::ptr add_cond_timer(time_t interval, std::function<void()> *cb, std::weak_ptr<void> cond, bool loop = false);

    bool is_valid_timer(Timer::ptr timer);
    void reset_timer(Timer::ptr timer, time_t interval);
    bool cancel_timer(Timer::ptr timer);
    void cancel_all_timer();

    time_t next_timer_left_time();
    void all_expired_cbs(std::vector<std::function<void()>> &cbs);
};

}  // namespace tiger

#endif
