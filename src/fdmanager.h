/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/15
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_FDMANAGER_H__
#define __TIGER_FDMANAGER_H__

#include <memory>
#include <vector>

#include "mutex.h"
#include "singleton.h"
#include "timer.h"

namespace tiger {

class FDEntity {
   private:
    bool m_is_init = false;
    bool m_is_sys_nonblock = false;
    bool m_is_user_nonblock = false;
    bool m_is_socket = false;
    bool m_is_closed = false;
    int m_fd = 0;
    uint64_t m_connect_timeout = 0;
    uint64_t m_recv_timeout = 0;
    uint64_t m_send_timeout = 0;
    TimerManager::Timer::ptr m_timer = nullptr;

   public:
    typedef std::shared_ptr<FDEntity> ptr;

    FDEntity(int fd);
    ~FDEntity(){};

   public:
    bool is_init() const { return m_is_init; }
    bool is_sys_nonblock() const { return m_is_sys_nonblock; }
    bool is_user_nonblock() const { return m_is_user_nonblock; }
    bool is_socket() const { return m_is_socket; }
    bool is_closed() const { return m_is_closed; }
    int fd() const { return m_fd; }
    uint64_t connect_timeout() const;
    uint64_t recv_timeout() const;
    uint64_t send_timeout() const;
    TimerManager::Timer::ptr timer() const { return m_timer; }

    void set_sys_nonblock(bool flag) { m_is_sys_nonblock = flag; }
    void set_user_nonblock(bool flag) { m_is_user_nonblock = flag; }
    void set_connect_timeout(uint64_t t) { m_connect_timeout = t; }
    void set_recv_timeout(uint64_t t) { m_recv_timeout = t; }
    void set_send_timeout(uint64_t t) { m_send_timeout = t; }
    void set_timer(TimerManager::Timer::ptr timer) { m_timer = timer; }

   public:
    bool init();
};

class FDManager {
   private:
    ReadWriteLock m_lock;
    std::vector<FDEntity::ptr> m_datas;

   public:
    typedef std::shared_ptr<FDManager> ptr;

    FDManager();
    ~FDManager(){};

   public:
    void del_fd(int fd);
    FDEntity::ptr add_fd(int fd);
    FDEntity::ptr get_fd(int fd, bool auto_create = false);
};

typedef Singleton<FDManager> SingletonFDManager;

}  // namespace tiger

#endif