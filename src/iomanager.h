/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_IOMANAGER_H__
#define __TIGER_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"

namespace tiger
{

class IOManager : public Scheduler, public TimerManager
{
  public:
    enum EventStatus
    {
        NONE = 0x0,
        READ = 0x001,
        WRITE = 0x004
    };

  private:
    typedef struct
    {
        struct EventContext
        {
            Scheduler::ptr scheduler;
            pid_t thread_id;
            Coroutine::ptr co;
            std::function<void()> cb;
        };
        MutexLock mutex;
        int fd = 0;
        EventStatus statuses = EventStatus::NONE;
        EventContext read;
        EventContext write;

        EventContext &get_event_context(const EventStatus status);
        void reset_event_context(EventContext &ctx);
        void trigger_event(const EventStatus status);
    } Context;

  private:
    int m_epfd = 0;
    int m_tickle_fds[2];
    ReadWriteLock m_lock;
    std::atomic<size_t> m_appending_event_cnt{0};
    std::vector<Context *> m_contexts;

  protected:
    void open_hook() override;
    void close_hook() override;
    void tickle() override;
    void idle() override;
    void on_timer_refresh() override;

  protected:
    void fd_context_resize(size_t size);

  public:
    typedef std::shared_ptr<IOManager> ptr;

    IOManager(const std::string &name = "", bool user_main_thread = false, size_t thread_cnt = 1);
    ~IOManager();

  public:
    static IOManager::ptr GetThreadIOM();

  public:
    bool add_event(int fd, EventStatus status, std::function<void()> cb = nullptr);
    bool del_event(int fd, EventStatus status);
    bool cancel_event(int fd, EventStatus status);
    bool cancel_all_event(int fd);
};

} // namespace tiger

#endif