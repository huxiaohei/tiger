/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "tcp_server.h"

#include "../config.h"

namespace tiger
{

static ConfigVar<int64_t>::ptr g_tcp_server_accept_timeout =
    Config::Lookup<int64_t>("tiger.servers.tcp.acceptTimeout", 6000, "tcp accept timeout");
static ConfigVar<int64_t>::ptr g_tcp_server_recv_timeout =
    Config::Lookup<int64_t>("tiger.servers.tcp.recvTimeout", 6000, "tcp accept timeout");

TCPServer::TCPServer(IOManager::ptr io_worker, IOManager::ptr accept_worker, const std::string &name)
    : m_io_worker(io_worker), m_accept_worker(accept_worker), m_accept_timeout(g_tcp_server_accept_timeout->val()),
      m_recv_timeout(g_tcp_server_recv_timeout->val()), m_name(name), m_is_stop(true)
{
}

TCPServer::~TCPServer()
{
    for (auto &it : m_sockets)
    {
        it->close();
    }
    m_sockets.clear();
}

bool TCPServer::bind(Address::ptr addr, bool is_ssl)
{
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails, is_ssl);
}

bool TCPServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails, bool is_ssl)
{
    for (auto &addr : addrs)
    {
        Socket::ptr socket = is_ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
        if (!socket->bind(addr))
        {
            TIGER_LOG_E(SYSTEM_LOG) << "[bind fail"
                                    << " addr:" << addr << " errno:" << strerror(errno) << "]";
            fails.push_back(addr);
            continue;
        }
        if (!socket->listen())
        {
            TIGER_LOG_E(SYSTEM_LOG) << "[listen fail"
                                    << " addr:" << addr << " errno:" << strerror(errno) << "]";
            fails.push_back(addr);
            continue;
        }
        m_sockets.push_back(socket);
    }
    if (!fails.empty())
    {
        m_sockets.clear();
        return false;
    }
    for (auto &it : m_sockets)
    {
        TIGER_LOG_I(SYSTEM_LOG) << "bind success ["
                                << "type: tcp"
                                << " name:" << m_name << " " << it << "]";
    }
    return true;
}

bool TCPServer::start()
{
    {
        MutexLock::Lock lock(m_lock);
        if (!m_is_stop)
        {
            return true;
        }
        m_is_stop = false;
    }
    for (auto &socket : m_sockets)
    {
        m_accept_worker->schedule(std::bind(&TCPServer::start_accept, shared_from_this(), socket));
    }
    return true;
}

void TCPServer::stop()
{
    MutexLock::Lock lock(m_lock);
    m_is_stop = true;
    auto self = shared_from_this();
    m_accept_worker->schedule([this, self]() {
        for (auto &socket : m_sockets)
        {
            socket->cancel_all();
            socket->close();
        }
        m_sockets.clear();
    });
}

bool TCPServer::load_certificates(const std::string &cert_file, const std::string &key_file)
{
    bool rst = true;
    for (auto &it : m_sockets)
    {
        auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(it);
        if (ssl_socket)
        {
            rst = ssl_socket->load_certificates(cert_file, key_file) ? rst : false;
        }
    }
    return rst;
}

void TCPServer::handle_client(Socket::ptr client)
{
    TIGER_LOG_I(SYSTEM_LOG) << "[handle client " << client << "]";
}

void TCPServer::start_accept(Socket::ptr socket)
{
    int64_t accept_timeout = m_accept_timeout;
    socket->set_recv_timeout(m_accept_timeout);
    while (!m_is_stop)
    {
        if (TIGER_UNLIKELY(accept_timeout != m_accept_timeout))
        {
            socket->set_recv_timeout(m_accept_timeout);
        }
        Socket::ptr client = socket->accept();
        if (client)
        {
            client->set_recv_timeout(m_accept_timeout);
            m_io_worker->schedule(std::bind(&TCPServer::handle_client, shared_from_this(), client));
        }
        else
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[accept timeout"
                                    << " addr:" << socket << "]";
        }
    }
}

} // namespace tiger