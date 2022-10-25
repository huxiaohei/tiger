/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_SOCKET_H__
#define __TIGER_SOCKET_H__

#include <netinet/tcp.h>

#include "address.h"

namespace tiger {

class Socket : public std::enable_shared_from_this<Socket> {
   protected:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_is_connected;
    Address::ptr m_local_addr;
    Address::ptr m_remote_addr;

   protected:
    virtual bool init(int sock);
    void init_sock();
    void new_sock();
    Address::ptr init_local_address();
    Address::ptr init_remote_address();

   public:
    typedef std::shared_ptr<Socket> ptr;

    Socket(int family, int type, int protocol = 0);
    virtual ~Socket();

    Socket(const Socket &) = delete;
    Socket(const Socket &&) = delete;
    Socket &operator=(const Socket &) = delete;

   public:
    static Socket::ptr CreateTCP(tiger::Address::ptr address);
    static Socket::ptr CreateUDP(tiger::Address::ptr address);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

   public:
    int get_family() const { return m_family; }
    int get_type() const { return m_type; }
    int get_protocol() const { return m_protocol; }
    bool is_connected() const { return m_is_connected; }
    bool is_valid() const { return m_sock != -1; }
    int get_socket() const { return m_sock; }
    virtual std::ostream &dump(std::ostream &os) const;
    virtual std::string to_string() const;

   public:
    int get_error();
    int64_t get_send_timeout();
    void set_send_timeout(int64_t timeout);
    int64_t get_recv_timeout();
    void set_recv_timeout(int64_t timeout);
    Address::ptr get_local_addr();
    Address::ptr get_remote_addr();
    bool get_option(int level, int optname, void *optval, socklen_t *optlen);
    bool set_option(int level, int optname, const void *optval, socklen_t optlen);

    virtual bool connect(const Address::ptr addr, uint16_t timeout_ms = -1);
    virtual bool reconnect(const uint64_t timeout_ms = -1);

    virtual bool bind(const Address::ptr addr);
    virtual bool listen(int backlog = SOMAXCONN);
    virtual Socket::ptr accept();
    virtual bool close();

    virtual int send(const void *buffer, size_t len, int flags = 0);
    virtual int send(const iovec *buffers, size_t len, int flags = 0);

    virtual int send_to(const void *buffer, size_t len, const Address::ptr to, int flags = 0);
    virtual int send_to(const iovec *buffers, size_t len, const Address::ptr to, int flags = 0);

    virtual int recv(void *buffer, size_t len, int flags = 0);
    virtual int recv(iovec *buffers, size_t len, int flags = 0);

    virtual int recv_from(void *buffer, size_t len, Address::ptr from, int flags = 0);
    virtual int recv_from(iovec *buffers, size_t len, Address::ptr from, int flags = 0);

    bool cancel_read();
    bool cancel_write();
    bool cancel_accept();
    bool cancel_all();

   public:
    friend std::ostream &operator<<(std::ostream &os, const Socket::ptr socket);
    friend std::ostream &operator<<(std::ostream &os, const Socket &socket);
};

class SSLSocket : public Socket {
   public:
    typedef std::shared_ptr<SSLSocket> ptr;
};

}  // namespace tiger

#endif
