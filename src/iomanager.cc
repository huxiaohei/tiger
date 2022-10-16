/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "iomanager.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <functional>
#include <vector>

#include "hook.h"

namespace tiger {

IOManager *IOManager::GetThreadIOM() {
    return dynamic_cast<IOManager *>(Scheduler::GetThreadScheduler());
}

IOManager::Context::EventContext &IOManager::Context::get_event_context(const EventStatus status) {
    return status == EventStatus::READ ? read : write;
}

void IOManager::Context::reset_event_context(EventContext &ctx) {
    ctx.thread_id = 0;
    ctx.co.reset();
    ctx.cb = nullptr;
    ctx.scheduler = nullptr;
}

void IOManager::Context::trigger_event(const EventStatus status) {
    if (!(statuses & status)) {
        TIGER_LOG_E(SYSTEM_LOG) << "tiger_event error:"
                                << "\n\tstatuses" << statuses
                                << "\n\tstatus" << status;
        return;
    }
    statuses = (EventStatus)(statuses & ~status);
    EventContext &ctx = get_event_context(status);
    if (ctx.co) {
        ctx.scheduler->schedule(&ctx.co, ctx.thread_id);
    } else {
        ctx.scheduler->schedule(&ctx.cb, ctx.thread_id);
    }
    reset_event_context(ctx);
}

IOManager::IOManager(const std::string &name, bool use_main_thread, size_t thread_cnt)
    : Scheduler(name, use_main_thread, thread_cnt) {
    m_epfd = epoll_create(1024);
    TIGER_ASSERT_WITH_INFO(m_epfd > 0, "epoll_create error");
    int rt = pipe(m_tickle_fds);
    TIGER_ASSERT_WITH_INFO(rt == 0, "pipe error");
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = m_tickle_fds[0];
    TIGER_ASSERT_WITH_INFO(!fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK), "fcntl error");
    TIGER_ASSERT_WITH_INFO(!epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickle_fds[0], &event), "epoll_ctl error");
    fd_context_resize(128);
}

IOManager::~IOManager() {
    cancel_all_timer();
    close(m_epfd);
    close(m_tickle_fds[0]);
    close(m_tickle_fds[1]);
    for (size_t i = 0; i < m_contexts.size(); ++i) {
        if (m_contexts[i]) delete m_contexts[i];
    }
}

void IOManager::open_hook() {
    enable_hook(true);
}

void IOManager::close_hook() {
    enable_hook(false);
}

void IOManager::tickle() {
    if (has_idel_thread()) {
        int rt = write(m_tickle_fds[1], "T", 1);
        TIGER_ASSERT_WITH_INFO(rt == 1, "write error");
    }
}

void IOManager::idle() {
    epoll_event *events = new epoll_event[256]();
    int rt = 0;
    std::vector<std::function<void()>> cbs;
    while (true) {
        rt = 0;
        do {
            static const int EPOLL_MAX_TIMEOUT = 5000;
            time_t next_time = next_timer_left_time();
            rt = epoll_wait(m_epfd, events, 256, next_time > EPOLL_MAX_TIMEOUT ? EPOLL_MAX_TIMEOUT : (int)next_time);
            if (rt > 0 || next_timer_left_time() <= 0 || is_stopping()) break;
        } while (true);
        all_expired_cbs(cbs);
        schedules(cbs.begin(), cbs.end());
        cbs.clear();
        for (int i = 0; i < rt; ++i) {
            epoll_event &event = events[i];
            if (event.data.fd == m_tickle_fds[0]) {
                uint8_t dummy;
                while (read(m_tickle_fds[0], &dummy, 1) == 1)
                    continue;
                continue;
            }
            Context *ctx = (Context *)event.data.ptr;
            MutexLock::Lock lock(ctx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= EPOLLIN | EPOLLOUT;
            }
            EventStatus real_status = EventStatus::NONE;
            if (event.events & EPOLLIN) {
                real_status = EventStatus::READ;
            }
            if (event.events & EPOLLOUT) {
                real_status = EventStatus::WRITE;
            }
            if ((EventStatus)(ctx->statuses & real_status) == EventStatus::NONE)
                continue;
            EventStatus left_status = (EventStatus)(ctx->statuses & ~real_status);
            int op = left_status ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_status;
            if (epoll_ctl(m_epfd, op, ctx->fd, &event)) {
                TIGER_LOG_E(SYSTEM_LOG) << "epoll_ctl error:"
                                        << "\n\tepfd:" << m_epfd
                                        << "\n\top:" << op
                                        << "\n\tfd:" << ctx->fd
                                        << "\n\terrno:" << strerror(errno);
                continue;
            }
            if (real_status & EventStatus::READ) {
                ctx->trigger_event(EventStatus::READ);
                --m_appending_event_cnt;
            }
            if (real_status & EventStatus::WRITE) {
                ctx->trigger_event(EventStatus::WRITE);
                --m_appending_event_cnt;
            }
        }
        {
            MutexLock::Lock lock(m_mutex);
            if (is_stopping() && m_tasks.size() == 0) {
                if (main_thread_id() == Thread::CurThreadId()) {
                    if (thread_cnt() == 0) break;
                } else {
                    break;
                }
            }
        }
        Coroutine::Yield();
    }
}

void IOManager::on_timer_refresh() {
    tickle();
}

void IOManager::fd_context_resize(size_t size) {
    if (size <= m_contexts.size()) return;
    m_contexts.resize(size);
    for (size_t i = 0; i < size; ++i) {
        if (!m_contexts[i]) {
            m_contexts[i] = new Context;
            m_contexts[i]->fd = i;
        }
    }
}

bool IOManager::add_event(int fd, EventStatus status, std::function<void()> cb) {
    ReadWriteLock::ReadLock rlock(m_lock);
    Context *ctx = nullptr;
    if ((int)m_contexts.size() > fd) {
        ctx = m_contexts[fd];
        rlock.unlock();
    } else {
        rlock.unlock();
        ReadWriteLock::WriteLock wlock(m_lock);
        fd_context_resize(fd * 1.5);
        ctx = m_contexts[fd];
    }
    MutexLock::Lock mlock(ctx->mutex);
    if (ctx->statuses & status) {
        TIGER_LOG_E(SYSTEM_LOG) << "add_event error:"
                                << "\n\tstatuses:" << ctx->statuses
                                << "\n\tstatus:" << status;
        return false;
    }
    int op = ctx->statuses ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event event;
    event.events = ctx->statuses | status | EPOLLET;
    event.data.ptr = ctx;
    if (epoll_ctl(m_epfd, op, fd, &event)) {
        TIGER_LOG_E(SYSTEM_LOG) << "epoll_ctl error:"
                                << "\n\tepfd:" << m_epfd
                                << "\n\top:" << op
                                << "\n\tfd:" << fd
                                << "\n\terrno:" << strerror(errno);
        return false;
    }
    ++m_appending_event_cnt;
    ctx->statuses = (EventStatus)(ctx->statuses | status);
    auto &event_context = ctx->get_event_context(status);
    event_context.scheduler = Scheduler::GetThreadScheduler();
    if (cb) {
        event_context.cb.swap(cb);
    } else {
        event_context.co = Coroutine::GetRunningCo();
        event_context.thread_id = Thread::CurThreadId();
    }
    return true;
}

bool IOManager::del_event(int fd, EventStatus status) {
    ReadWriteLock::ReadLock rlock(m_lock);
    if ((int)m_contexts.size() <= fd) {
        TIGER_LOG_E(SYSTEM_LOG) << "del_event error:"
                                << "\n\tfd:" << fd
                                << "\n\tsize:" << m_contexts.size();
        return false;
    }
    auto ctx = m_contexts[fd];
    if (!(ctx->statuses & status)) {
        TIGER_LOG_E(SYSTEM_LOG) << "del_event error:"
                                << "\n\tfd:" << fd
                                << "\n\tstatuses:" << ctx->statuses
                                << "\n\tstatus:" << status;
        return false;
    }
    rlock.unlock();
    MutexLock mlock(ctx->mutex);
    EventStatus new_status = (EventStatus)(ctx->statuses & ~status);
    int op = new_status ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event event;
    event.events = EPOLLET | new_status;
    event.data.ptr = ctx;
    if (epoll_ctl(m_epfd, op, fd, &event)) {
        TIGER_LOG_E(SYSTEM_LOG) << "epoll_ctl error:"
                                << "\n\tepfd:" << m_epfd
                                << "\n\top:" << op
                                << "\n\tfd:" << fd
                                << "\n\terrno:" << strerror(errno);
        return false;
    }
    --m_appending_event_cnt;
    ctx->statuses = new_status;
    auto &event_context = ctx->get_event_context(new_status);
    ctx->reset_event_context(event_context);
    return true;
}

bool IOManager::cancel_event(int fd, EventStatus status) {
    ReadWriteLock::ReadLock rlock(m_lock);
    if ((int)m_contexts.size() <= fd) {
        TIGER_LOG_E(SYSTEM_LOG) << "cancel_event error:"
                                << "\n\tfd:" << fd
                                << "\n\tsize:" << m_contexts.size();
        return false;
    }
    auto ctx = m_contexts[fd];
    if (!(ctx->statuses & status)) {
        TIGER_LOG_E(SYSTEM_LOG) << "cancel_event error:"
                                << "\n\tfd:" << fd
                                << "\n\tstatuses:" << ctx->statuses
                                << "\n\tstatus:" << status;
        return false;
    }
    rlock.unlock();

    MutexLock::Lock mlock(ctx->mutex);
    EventStatus new_status = (EventStatus)(ctx->statuses & ~status);
    int op = new_status ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event event;
    event.events = EPOLLET | new_status;
    event.data.ptr = ctx;
    if (epoll_ctl(m_epfd, op, fd, &event)) {
        TIGER_LOG_E(SYSTEM_LOG) << "epoll_ctl error:"
                                << "\n\tepfd:" << m_epfd
                                << "\n\top:" << op
                                << "\n\tfd:" << fd
                                << "\n\terrno:" << strerror(errno);
        return false;
    }
    --m_appending_event_cnt;
    ctx->trigger_event(status);
    return true;
}

bool IOManager::cancel_all_event(int fd) {
    ReadWriteLock::ReadLock rlock(m_lock);
    if ((int)m_contexts.size() <= fd) {
        TIGER_LOG_E(SYSTEM_LOG) << "cancel_event error:"
                                << "\n\tfd:" << fd
                                << "\n\tsize:" << m_contexts.size();
        return false;
    }
    auto ctx = m_contexts[fd];
    rlock.unlock();
    MutexLock::Lock mlock(ctx->mutex);
    epoll_event event;
    event.events = EventStatus::NONE;
    event.data.ptr = ctx;
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &event)) {
        TIGER_LOG_E(SYSTEM_LOG) << "epoll_ctl error:"
                                << "\n\tepfd:" << m_epfd
                                << "\n\top:" << EPOLL_CTL_DEL
                                << "\n\tfd:" << fd
                                << "\n\terrno:" << strerror(errno);
        return false;
    }
    if (ctx->statuses & EventStatus::READ) {
        ctx->trigger_event(EventStatus::READ);
        --m_appending_event_cnt;
    }
    if (ctx->statuses & EventStatus::WRITE) {
        ctx->trigger_event(EventStatus::WRITE);
        --m_appending_event_cnt;
    }
    TIGER_ASSERT_WITH_INFO(ctx->statuses == EventStatus::NONE, "cancel_all_event error");
    return true;
}

}  // namespace tiger