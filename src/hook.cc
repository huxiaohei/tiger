/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/13
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "hook.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#include "fdmanager.h"
#include "iomanager.h"
#include "thread.h"

namespace tiger {

#define HOOK_FUNC(XX) \
    XX(sleep);        \
    XX(usleep);       \
    XX(nanosleep);    \
    XX(socket);       \
    XX(connect);      \
    XX(accept);       \
    XX(read);         \
    XX(readv);        \
    XX(recv);         \
    XX(recvfrom);     \
    XX(recvmsg);      \
    XX(write);        \
    XX(writev);       \
    XX(send);         \
    XX(sendto);       \
    XX(sendmsg);      \
    XX(close);        \
    XX(fcntl);        \
    XX(ioctl);        \
    XX(getsockopt);   \
    XX(setsockopt)

void __init() {
    static bool is_init = false;
    if (is_init) return;
    is_init = true;
#define XX(name) name##_f = (name##_func)dlsym(RTLD_NEXT, #name);
    HOOK_FUNC(XX);
#undef XX
}

struct __Hook {
    __Hook() { __init(); }
};

static __Hook s__hook;

static thread_local bool t__enable_hook = false;

void enable_hook(bool hook) {
    t__enable_hook = hook;
}

bool __enable_hook() {
    return t__enable_hook;
}

}  // namespace tiger

typedef struct {
    bool canceled = false;
} SocketIoState;

template <typename OrgFunc, typename... Args>
static ssize_t do_socket_io(int fd, OrgFunc func, const char *hook_func_name,
                            tiger::IOManager::EventStatus status, Args &&...args) {
    if (!tiger::__enable_hook()) return func(fd, std::forward<Args>(args)...);
    auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(fd);
    if (!fd_entity) return func(fd, std::forward<Args>(args)...);
    if (fd_entity->is_closed()) {
        errno = EBADF;
        return -1;
    }
    if (!fd_entity->is_socket() || fd_entity->is_user_nonblock()) {
        return func(fd, std::forward<Args>(args)...);
    }
    int timeout = -1;
    if (status & tiger::IOManager::EventStatus::READ) {
        timeout = fd_entity->recv_timeout();
    } else if (status & tiger::IOManager::EventStatus::WRITE) {
        timeout = fd_entity->send_timeout();
    } else {
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "[iomanager event status not found"
                                       << " status:" << status
                                       << " func:" << hook_func_name << "]";
    }
    auto state = std::make_shared<SocketIoState>();
    ssize_t n = -1;
    do {
        n = func(fd, std::forward<Args>(args)...);
        if (n == -1 && errno == EAGAIN) {
            auto iom = tiger::IOManager::GetThreadIOM();
            tiger::TimerManager::Timer::ptr timer;
            std::weak_ptr<SocketIoState> week_state(state);
            if (timeout >= 0) {
                timer = iom->add_cond_timer(
                    timeout, [week_state, fd, iom, status]() {
                        auto _week_state = week_state.lock();
                        if (!_week_state || _week_state->canceled) {
                            return;
                        }
                        _week_state->canceled = true;
                        iom->cancel_event(fd, status);
                    },
                    week_state, false);
            }
            if (iom->add_event(fd, status)) {
                // 问题：多线程环境下，协程Yield之前事件被触发了怎么办？
                // 解决：
                //  1.事件触发的时候必定会在此线程中被唤醒
                //  2.在调度器中如果发现任务池中有指定线程任务，但不是本线程则发起tickle唤醒一个idle进程
                //  3.在线程进入idle状态的时候，如果之前tickle过，则重置tickle状态
                tiger::Coroutine::Yield();
                iom->cancel_timer(timer);
                if (state->canceled) {
                    errno = ETIMEDOUT;
                    return -1;
                }
                state->canceled = false;
                continue;
            } else {
                iom->cancel_timer(timer);
                TIGER_LOG_E(tiger::SYSTEM_LOG) << "[iomanager add event error"
                                               << " status:" << status
                                               << " hookName:" << hook_func_name << "]";
                return -1;
            }
        }
    } while (n == -1 && (errno == EINTR || errno == EAGAIN));
    return n;
}

extern "C" {

#define XX(name) name##_func name##_f = nullptr;
HOOK_FUNC(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!tiger::__enable_hook()) return sleep_f(seconds);
    pid_t t = tiger::Thread::CurThreadId();
    auto iom = tiger::IOManager::GetThreadIOM();
    auto co = tiger::Coroutine::GetRunningCo();
    iom->add_timer(
        seconds * 1000, [iom, co, t]() {
            iom->schedule(co, t);
        },
        false);
    tiger::Coroutine::Yield();
    return 0;
}

int usleep(useconds_t usec) {
    if (!tiger::__enable_hook()) return usleep_f(usec);
    pid_t t = tiger::Thread::CurThreadId();
    auto iom = tiger::IOManager::GetThreadIOM();
    auto co = tiger::Coroutine::GetRunningCo();
    iom->add_timer(
        usec, [iom, co, t]() {
            iom->schedule(co, t);
        },
        false);
    tiger::Coroutine::Yield();
    return 0;
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp) {
    if (!tiger::__enable_hook()) return nanosleep(rqtp, rmtp);
    time_t usc = rqtp->tv_sec * 1000 + rqtp->tv_nsec / 10000000;
    auto t = tiger::Thread::CurThreadId();
    auto iom = tiger::IOManager::GetThreadIOM();
    auto co = tiger::Coroutine::GetRunningCo();
    iom->add_timer(
        usc, [iom, co, t]() {
            iom->schedule(co, t);
        },
        false);
    tiger::Coroutine::Yield();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if (!tiger::__enable_hook()) return socket_f(domain, type, protocol);
    int fd = socket_f(domain, type, protocol);
    if (-1 == fd) {
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "[create socket fail]";
        return fd;
    }
    return tiger::SingletonFDManager::Instance()->get_fd(fd, true)->fd();
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (!tiger::__enable_hook()) return connect_f(sockfd, addr, addrlen);
    auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(sockfd);
    if (!fd_entity || fd_entity->is_closed()) {
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "[socket connect fail]";
        errno = EBADF;
        return -1;
    }
    if (!fd_entity->is_socket()) {
        return connect_f(sockfd, addr, addrlen);
    }
    if (fd_entity->is_user_nonblock()) {
        return connect_f(sockfd, addr, addrlen);
    }
    int n = connect_f(sockfd, addr, addrlen);
    if (n == 0) return n;
    if (n != -1 || errno != EINPROGRESS) return n;
    auto iom = tiger::IOManager::GetThreadIOM();
    auto state = std::make_shared<SocketIoState>();
    std::weak_ptr<SocketIoState> week_state(state);
    tiger::TimerManager::Timer::ptr timer;
    if (fd_entity->connect_timeout() > 0) {
        timer = iom->add_cond_timer(
            fd_entity->connect_timeout(), [week_state, sockfd, iom]() {
                auto _week_state = week_state.lock();
                if (!_week_state || _week_state->canceled) return;
                _week_state->canceled = true;
                iom->cancel_event(sockfd, tiger::IOManager::EventStatus::WRITE);
            },
            week_state, false);
    }
    if (iom->add_event(sockfd, tiger::IOManager::EventStatus::WRITE)) {
        tiger::Coroutine::Yield();
        iom->cancel_timer(timer);
        if (state->canceled) {
            errno = ETIMEDOUT;
            return -1;
        }
    } else {
        iom->cancel_timer(timer);
        state->canceled = true;
    }
    int err = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len)) {
        return -1;
    }
    if (!err) {
        return 0;
    } else {
        errno = err;
        return -1;
    }
}

int accept(int socket, struct sockaddr *address, socklen_t *address_len) {
    int fd = do_socket_io(socket, accept_f, "accept",
                          tiger::IOManager::EventStatus::READ, address, address_len);
    if (fd >= 0) {
        tiger::SingletonFDManager::Instance()->get_fd(fd, true);
    }
    return fd;
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
    return do_socket_io(fildes, read_f, "read",
                        tiger::IOManager::EventStatus::READ, buf, nbyte);
}

ssize_t readv(int fildes, const struct iovec *iov, int iovcnt) {
    return do_socket_io(fildes, readv_f, "readv",
                        tiger::IOManager::EventStatus::READ, iov, iovcnt);
}

ssize_t recv(int socket, void *buffer, size_t length, int flags) {
    return do_socket_io(socket, recv_f, "recv",
                        tiger::IOManager::EventStatus::READ, buffer, length, flags);
}

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags,
                 struct sockaddr *address, socklen_t *address_len) {
    return do_socket_io(socket, recvfrom_f, "recvfrom",
                        tiger::IOManager::EventStatus::READ, buffer, length, flags,
                        address, address_len);
}

ssize_t recvmsg(int socket, struct msghdr *message, int flags) {
    return do_socket_io(socket, recvmsg_f, "recvmsg",
                        tiger::IOManager::EventStatus::READ, message, flags);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
    return do_socket_io(fildes, write_f, "write",
                        tiger::IOManager::EventStatus::WRITE, buf, nbyte);
}

ssize_t writev(int fildes, const struct iovec *iov, int iovcnt) {
    return do_socket_io(fildes, writev_f, "writev",
                        tiger::IOManager::EventStatus::WRITE, iov, iovcnt);
}

ssize_t send(int socket, const void *buffer, size_t length, int flags) {
    return do_socket_io(socket, send_f, "send",
                        tiger::IOManager::EventStatus::WRITE, buffer, length, flags);
}

ssize_t sendto(int socket, const void *message, size_t length,
               int flags, const struct sockaddr *dest_addr,
               socklen_t dest_len) {
    return do_socket_io(socket, sendto_f, "sendto",
                        tiger::IOManager::EventStatus::WRITE, message, length,
                        flags, dest_addr, dest_len);
}

ssize_t sendmsg(int socket, const struct msghdr *message, int flags) {
    return do_socket_io(socket, sendmsg_f, "sendmsg",
                        tiger::IOManager::EventStatus::WRITE, message, flags);
}

int close(int fildes) {
    if (!tiger::__enable_hook()) {
        return close_f(fildes);
    }
    auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(fildes);
    if (fd_entity) {
        auto iom = tiger::IOManager::GetThreadIOM();
        if (iom) {
            iom->cancel_all_event(fildes);
        }
        tiger::SingletonFDManager::Instance()->del_fd(fildes);
    }
    return close_f(fildes);
}

int fcntl(int fildes, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(ap, int);
            va_end(ap);
            auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(fildes);
            if (!fd_entity || !fd_entity->is_socket() || fd_entity->is_closed()) {
                return fcntl_f(fildes, cmd, arg);
            }
            fd_entity->set_user_nonblock(arg & O_NONBLOCK);
            if (fd_entity->is_sys_nonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= !O_NONBLOCK;
            }
            return fcntl_f(fildes, cmd, arg);
        }
        case F_GETFL: {
            va_end(ap);
            int flag = fcntl_f(fildes, cmd);
            auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(fildes);
            if (!fd_entity || !fd_entity->is_socket() || fd_entity->is_closed()) {
                return flag;
            }
            if (fd_entity->is_user_nonblock()) {
                return flag | O_NONBLOCK;
            } else {
                return flag & ~O_NONBLOCK;
            }
        }
        case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
        case F_DUPFD_CLOEXEC:
#endif
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
#ifdef F_ADD_SEALS
        case F_ADD_SEALS:
#endif

        case F_NOTIFY: {
            int arg = va_arg(ap, int);
            va_end(ap);
            return fcntl_f(fildes, cmd, arg);
        }
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(ap);
            return fcntl_f(fildes, cmd);
        }
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock *arg = va_arg(ap, struct flock *);
            va_end(ap);
            return fcntl_f(fildes, cmd, arg);
        }
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock *arg = va_arg(ap, struct f_owner_exlock *);
            va_end(ap);
            return fcntl_f(fildes, cmd, arg);
        }
        default:
            va_end(ap);
            return fcntl_f(fildes, cmd);
    }
}

int ioctl(int fd, unsigned long request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void *);
    va_end(va);
    if (FIONBIO == request) {
        bool user_nonblock = !!*(int *)arg;
        auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(fd);
        if (!fd_entity || !fd_entity->is_socket() || fd_entity->is_closed()) {
            return ioctl_f(fd, request, arg);
        }
        fd_entity->set_user_nonblock(user_nonblock);
    }
    return ioctl_f(fd, request, arg);
}

int getsockopt(int socket, int level, int option_name,
               void *option_value, socklen_t *option_len) {
    return getsockopt_f(socket, level, option_name, option_value, option_len);
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen) {
    if (tiger::__enable_hook()) return setsockopt_f(sockfd, level, optname, optval, optlen);
    if (level == SOL_SOCKET && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)) {
        auto fd_entity = tiger::SingletonFDManager::Instance()->get_fd(sockfd);
        if (fd_entity) {
            const timeval *v = (const timeval *)optval;
            if (optname == SO_RCVTIMEO) {
                fd_entity->set_recv_timeout(v->tv_sec * 1000 + v->tv_usec / 1000);
            } else {
                fd_entity->set_send_timeout(v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}