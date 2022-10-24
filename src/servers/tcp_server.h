/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_TCP_SERVER_H__
#define __TIGER_TCP_SERVER_H__

#include "../iomanager.h"
#include "../socket.h"

namespace tiger {

class TCPServer : public std::enable_shared_from_this<TCPServer> {
   private:
    MutexLock m_lock;

   protected:
    std::vector<Socket::ptr> m_sockets;
    IOManager::ptr m_io_worker;
    IOManager::ptr m_accept_worker;
    int64_t m_accept_timeout;
    int64_t m_recv_timeout;
    std::string m_name;
    bool m_is_stop;

   protected:
    virtual void handle_client(Socket::ptr client);
    virtual void start_accept(Socket::ptr socket);

   public:
    typedef std::shared_ptr<TCPServer> ptr;

    TCPServer(IOManager::ptr io_worker = IOManager::GetThreadIOM(),
              IOManager::ptr accept_worker = IOManager::GetThreadIOM(),
              const std::string &name = "");
    virtual ~TCPServer();

   public:
    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr> &addrs,
                      std::vector<Address::ptr> &fails);

    virtual bool start();
    virtual void stop();

   public:
    int64_t get_accept_timeout() const { return m_accept_timeout; }
    int64_t get_recv_timeout() const { return m_recv_timeout; }
    std::string get_name() const { return m_name; }
    bool is_stop() const { return m_is_stop; }

    void set_accept_timeout(int64_t timeout_ms) { m_accept_timeout = timeout_ms; }
    void set_recv_timeout(int64_t timeout_ms) { m_recv_timeout = timeout_ms; }
    void set_name(const std::string &name) { m_name = name; }
};

}  // namespace tiger

#endif