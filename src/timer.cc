/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "timer.h"

#include <atomic>

#include "config.h"
#include "util.h"

namespace tiger {

static std::atomic<size_t> s_timer_id{0};

TimerManager::Timer::Timer(time_t ms, bool loop, std::function<void()> cb)
    : m_interval(ms), m_loop(loop) {
    m_id = ++s_timer_id;
    m_next_time = Millisecond() + m_interval;
    m_cb.swap(cb);
}

bool TimerManager::Timer::Comparator::operator()(const Timer::ptr &l, const Timer::ptr &r) const {
    if (!r) return true;
    if (!l) return false;
    if (l->m_next_time > r->m_next_time) return false;
    if (l->m_next_time < r->m_next_time) return true;
    return l.get() < r.get();
}

TimerManager::TimerManager() {
}

TimerManager::~TimerManager() {
}

TimerManager::Timer::ptr TimerManager::add_timer(time_t interval, std::function<void()> cb, bool loop) {
    auto timer = std::make_shared<Timer>(interval, loop, cb);
    ReadWriteLock::WriteLock lock(m_lock);
    m_timers.insert(timer);
    if (m_timers.begin() == m_timers.find(timer)) {
        on_timer_refresh();
    }
    return timer;
}

TimerManager::Timer::ptr TimerManager::add_timer(time_t interval, std::function<void()> *cb, bool loop) {
    return add_timer(interval, *cb, loop);
}

static void cond_timer_callback(std::weak_ptr<void> cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = cond.lock();
    if (tmp) cb();
}

TimerManager::Timer::ptr TimerManager::add_cond_timer(time_t interval, std::function<void()> cb, std::weak_ptr<void> cond, bool loop) {
    return add_timer(interval, std::bind(&cond_timer_callback, cond, cb));
}

TimerManager::Timer::ptr TimerManager::add_cond_timer(time_t interval, std::function<void()> *cb, std::weak_ptr<void> cond, bool loop) {
    return add_cond_timer(interval, *cb, cond, loop);
}

bool TimerManager::is_valid_timer(Timer::ptr timer) {
    ReadWriteLock::ReadLock lock(m_lock);
    return !(m_timers.find(timer) == m_timers.end());
}

void TimerManager::reset_timer(Timer::ptr timer, time_t interval) {
    if (timer->m_interval == interval) return;
    ReadWriteLock::WriteLock lock(m_lock);
    timer->m_interval = interval;
    if (m_timers.find(timer) == m_timers.end()) {
        m_timers.insert(timer);
    } else {
        m_timers.erase(timer);
        m_timers.insert(timer);
    }
    if (m_timers.find(timer) == m_timers.begin()) {
        on_timer_refresh();
    }
}

bool TimerManager::cancel_timer(Timer::ptr timer) {
    ReadWriteLock::WriteLock lock(m_lock);
    if (!timer) return false;
    if (m_timers.find(timer) == m_timers.end()) {
        return false;
    }
    bool need_refresh = m_timers.begin() == m_timers.find(timer);
    m_timers.erase(timer);
    timer->m_cb = nullptr;
    if (need_refresh) {
        on_timer_refresh();
    }
    return true;
}

void TimerManager::cancel_all_timer() {
    ReadWriteLock::WriteLock lock(m_lock);
    while (m_timers.begin() != m_timers.end()) {
        (*m_timers.begin())->m_cb = nullptr;
        m_timers.erase(m_timers.begin());
    }
    on_timer_refresh();
}

time_t TimerManager::next_timer_left_time() {
    ReadWriteLock::ReadLock lock(m_lock);
    if (m_timers.empty()) return ~0;
    auto n_ms = Millisecond();
    if (n_ms >= (*m_timers.begin())->m_next_time) {
        return 0;
    } else {
        return (*m_timers.begin())->m_next_time - n_ms;
    }
}

void TimerManager::all_expired_cbs(std::vector<std::function<void()>> &cbs) {
    {
        ReadWriteLock::ReadLock lock(m_lock);
        if (m_timers.empty()) return;
    }
    ReadWriteLock::WriteLock lock(m_lock);
    auto n_ms = Millisecond();
    std::vector<TimerManager::Timer::ptr> expired_timers;
    for (auto &timer : m_timers) {
        if (timer->m_next_time > n_ms) break;
        expired_timers.push_back(timer);
    }
    for (auto &timer : expired_timers) {
        m_timers.erase(timer);
        cbs.push_back(timer->m_cb);
        if (timer->m_loop) {
            timer->m_next_time = timer->m_interval + n_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

}  // namespace tiger
