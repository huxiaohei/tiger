/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/15
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "fdmanager.h"

#include <sys/stat.h>

#include "config.h"
#include "hook.h"

namespace tiger {

static ConfigVar<uint64_t>::ptr g_tcp_connect_timeout = Config::Lookup<uint64_t>("tiger.tcp.connectTimeout", 6000, "tcp connect timeout");
static ConfigVar<uint64_t>::ptr g_tcp_recv_timeout = Config::Lookup<uint64_t>("tiger.tcp.recvTimeout", 6000, "tcp recv timeout");
static ConfigVar<uint64_t>::ptr g_tcp_send_timeout = Config::Lookup<uint64_t>("tiger.tcp.sendTimeout", 6000, "tcp send timeout");

FDEntity::FDEntity(int fd)
    : m_fd(fd) {
    init();
}

bool FDEntity::init() {
    if (m_is_init) return false;
    m_connect_timeout = 0;
    m_recv_timeout = 0;
    m_send_timeout = 0;
    struct stat fd_stat;
    if (-1 == fstat(m_fd, &fd_stat)) {
        m_is_init = false;
        m_is_socket = false;
    } else {
        m_is_init = true;
        m_is_socket = S_ISSOCK(fd_stat.st_mode);
    }
    if (m_is_socket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_is_sys_nonblock = true;
    } else {
        m_is_sys_nonblock = false;
    }
    m_is_user_nonblock = false;
    m_is_closed = false;
    return m_is_init;
}

uint64_t FDEntity::connect_timeout() const {
    return m_connect_timeout == 0 ? g_tcp_connect_timeout->val() : m_connect_timeout;
}

uint64_t FDEntity::recv_timeout() const {
    return m_recv_timeout == 0 ? g_tcp_recv_timeout->val() : m_recv_timeout;
}

uint64_t FDEntity::send_timeout() const {
    return m_send_timeout == 0 ? g_tcp_send_timeout->val() : m_send_timeout;
}

FDManager::FDManager() {
    m_datas.resize(60);
}

void FDManager::del_fd(int fd) {
    ReadWriteLock::WriteLock lock(m_lock);
    if (fd >= (int)m_datas.size()) return;
    m_datas[fd].reset();
}

FDEntity::ptr FDManager::add_fd(int fd) {
    ReadWriteLock::WriteLock lock(m_lock);
    if (fd >= (int)m_datas.size()) {
        m_datas.resize((m_datas.size() * 1.5) > fd ? m_datas.size() * 1.5 : fd + 1);
    }
    m_datas[fd] = std::make_shared<FDEntity>(fd);
    return m_datas[fd];
}

FDEntity::ptr FDManager::get_fd(int fd, bool auto_create) {
    {
        ReadWriteLock::ReadLock lock(m_lock);
        if (fd >= (int)m_datas.size()) {
            if (!auto_create) return nullptr;
        } else {
            if (m_datas[fd]) return m_datas[fd];
            if (!auto_create) return nullptr;
        }
    }
    return add_fd(fd);
}

}  // namespace tiger