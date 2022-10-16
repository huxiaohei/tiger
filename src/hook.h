/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/13
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HOOK_H__
#define __TIGER_HOOK_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace tiger {

void enable_hook(bool hook);

}  // namespace tiger

extern "C" {

typedef unsigned int (*sleep_func)(unsigned int seconds);
extern sleep_func sleep_f;

typedef int (*usleep_func)(useconds_t usec);
extern usleep_func usleep_f;

typedef int (*nanosleep_func)(const struct timespec *rqtp, struct timespec *rmtp);
extern nanosleep_func nanosleep_f;

typedef int (*socket_func)(int domain, int type, int protocol);
extern socket_func socket_f;

typedef int (*connect_func)(int socket, const struct sockaddr *address,
                            socklen_t address_len);
extern connect_func connect_f;

typedef int (*accept_func)(int socket, struct sockaddr *address,
                           socklen_t *address_len);
extern accept_func accept_f;

typedef ssize_t (*read_func)(int fildes, void *buf, size_t nbyte);
extern read_func read_f;

typedef ssize_t (*readv_func)(int fildes, const struct iovec *iov, int iovcnt);
extern readv_func readv_f;

typedef ssize_t (*recv_func)(int socket, void *buffer, size_t length, int flags);
extern recv_func recv_f;

typedef ssize_t (*recvfrom_func)(int socket, void *buffer, size_t length,
                                 int flags, struct sockaddr *address,
                                 socklen_t *address_len);
extern recvfrom_func recvfrom_f;

typedef ssize_t (*recvmsg_func)(int socket, struct msghdr *message, int flags);
extern recvmsg_func recvmsg_f;

typedef ssize_t (*write_func)(int fildes, const void *buf, size_t nbyte);
extern write_func write_f;

typedef ssize_t (*writev_func)(int fildes, const struct iovec *iov, int iovcnt);
extern writev_func writev_f;

typedef ssize_t (*send_func)(int socket, const void *buffer, size_t length, int flags);
extern send_func send_f;

typedef ssize_t (*sendto_func)(int socket, const void *message, size_t length,
                               int flags, const struct sockaddr *dest_addr,
                               socklen_t dest_len);
extern sendto_func sendto_f;

typedef ssize_t (*sendmsg_func)(int socket, const struct msghdr *message, int flags);
extern sendmsg_func sendmsg_f;

typedef int (*close_func)(int fildes);
extern close_func close_f;

typedef int (*fcntl_func)(int fildes, int cmd, ...);
extern fcntl_func fcntl_f;

typedef int (*ioctl_func)(int fd, unsigned long request, ...);
;
extern ioctl_func ioctl_f;

typedef int (*getsockopt_func)(int socket, int level, int option_name,
                               void *option_value, socklen_t *option_len);
extern getsockopt_func getsockopt_f;

typedef int (*setsockopt_func)(int socket, int level, int option_name,
                               const void *option_value, socklen_t option_len);
extern setsockopt_func setsockopt_f;
}

#endif
