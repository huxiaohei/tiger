/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "socket.h"

#include "fdmanager.h"
#include "hook.h"
#include "iomanager.h"
#include "macro.h"

namespace tiger
{

Socket::ptr Socket::CreateTCP(tiger::Address::ptr address)
{
    Socket::ptr socket = std::make_shared<Socket>(address->get_family(), SOCK_STREAM, 0);
    return socket;
}

Socket::ptr Socket::CreateUDP(tiger::Address::ptr address)
{
    Socket::ptr socket = std::make_shared<Socket>(address->get_family(), SOCK_DGRAM, 0);
    socket->new_sock();
    socket->m_is_connected = true;
    return socket;
}

Socket::ptr Socket::CreateTCPSocket()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_INET, SOCK_STREAM, 0);
    return socket;
}

Socket::ptr Socket::CreateUDPSocket()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_INET, SOCK_DGRAM, 0);
    socket->new_sock();
    socket->m_is_connected = true;
    return socket;
}

Socket::ptr Socket::CreateTCPSocket6()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_INET6, SOCK_STREAM, 0);
    return socket;
}

Socket::ptr Socket::CreateUDPSocket6()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_INET6, SOCK_DGRAM, 0);
    socket->new_sock();
    socket->m_is_connected = true;
    return socket;
}

Socket::ptr Socket::CreateUnixTCPSocket()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_UNIX, SOCK_STREAM, 0);
    return socket;
}

Socket::ptr Socket::CreateUnixUDPSocket()
{
    Socket::ptr socket = std::make_shared<Socket>(AF_UNIX, SOCK_DGRAM, 0);
    return socket;
}

Socket::Socket(int family, int type, int protocol)
    : m_sock(-1), m_family(family), m_type(type), m_protocol(protocol), m_is_connected(false)
{
}

Socket::~Socket()
{
    close();
}

bool Socket::init(int sock)
{
    FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(sock);
    if (entity && entity->is_socket() && !entity->is_closed())
    {
        m_sock = sock;
        m_is_connected = true;
        init_sock();
        init_local_address();
        init_remote_address();
        return true;
    }
    return false;
}

void Socket::init_sock()
{
    int val = 1;
    set_option(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (m_type == SOCK_STREAM && m_family != AF_UNIX)
    {
        set_option(IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    }
}

void Socket::new_sock()
{
    m_sock = socket(m_family, m_type, m_protocol);
    if (TIGER_LIKELY(m_sock != -1))
    {
        init_sock();
    }
    else
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[new_socket fail"
                                << " family:" << m_family << " type:" << m_type << " protocol:" << m_protocol << "]";
    }
}

Address::ptr Socket::init_local_address()
{
    if (m_local_addr)
        return m_local_addr;
    Address::ptr rst;
    switch (m_family)
    {
    case AF_INET:
        rst = std::make_shared<IPv4Address>();
        break;
    case AF_INET6:
        rst = std::make_shared<IPv6Address>();
        break;
    case AF_UNIX:
        rst = std::make_shared<UnixAddress>();
        break;
    default:
        rst = std::make_shared<UnknownAddress>(m_family);
        break;
    }
    socklen_t addr_len = rst->get_addr_len();
    if (getsockname(m_sock, rst->get_addr(), &addr_len))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[getcockname fail"
                                << " sock:" << m_sock << " errno:" << strerror(errno) << "]";
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX)
    {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(rst);
        addr->set_addr_len(addr_len);
    }
    m_local_addr = rst;
    return m_local_addr;
}

Address::ptr Socket::init_remote_address()
{
    if (m_remote_addr)
        return m_remote_addr;
    Address::ptr rst;
    switch (m_family)
    {
    case AF_INET:
        rst = std::make_shared<IPv4Address>();
        break;
    case AF_INET6:
        rst = std::make_shared<IPv6Address>();
        break;
    case AF_UNIX:
        rst = std::make_shared<UnixAddress>();
        break;
    default:
        rst = std::make_shared<UnknownAddress>(m_family);
        break;
    }
    socklen_t addr_len = rst->get_addr_len();
    if (getpeername(m_sock, rst->get_addr(), &addr_len))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[getpeername fail"
                                << " sock:" << m_sock << " errno:" << strerror(errno) << "]";
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX)
    {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(rst);
        addr->set_addr_len(addr_len);
    }
    m_remote_addr = rst;
    return m_remote_addr;
}

int Socket::get_error()
{
    int err = 0;
    socklen_t len = sizeof(err);
    if (!get_option(SOL_SOCKET, SO_ERROR, &err, &len))
    {
        err = errno;
    }
    return err;
}

int64_t Socket::get_send_timeout()
{
    FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(m_sock);
    if (entity)
    {
        return entity->send_timeout();
    }
    return -1;
}

void Socket::set_send_timeout(int64_t timeout)
{
    FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(m_sock);
    if (entity)
    {
        entity->set_send_timeout(timeout);
    }
}

int64_t Socket::get_recv_timeout()
{
    FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(m_sock);
    if (entity)
    {
        return entity->recv_timeout();
    }
    return -1;
}

void Socket::set_recv_timeout(int64_t timeout)
{
    FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(m_sock);
    if (entity)
    {
        entity->set_recv_timeout(timeout);
    }
}

Address::ptr Socket::get_local_addr()
{
    if (m_local_addr)
        return m_local_addr;
    return init_local_address();
}

Address::ptr Socket::get_remote_addr()
{
    if (m_remote_addr)
        return m_remote_addr;
    return init_remote_address();
}

bool Socket::get_option(int level, int optname, void *optval, socklen_t *optlen)
{
    int rt = getsockopt(m_sock, level, optname, optval, optlen);
    if (rt)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[getsockopt fail"
                                << " sock:" << m_sock << " level:" << level << " option:" << optname
                                << " optval:" << optval << " errno:" << strerror(errno) << "]";
        return false;
    }
    return true;
}

bool Socket::set_option(int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(m_sock, level, optname, optval, optlen))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[setoption fail"
                                << " sock:" << m_sock << " level:" << level << " option:" << optname
                                << " optval:" << optval << " errno:" << strerror(errno) << "]";
        return false;
    }
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms)
{
    if (!is_valid())
    {
        new_sock();
        if (TIGER_UNLIKELY(!is_valid()))
        {
            return false;
        }
    }
    if (TIGER_UNLIKELY(addr->get_family() != m_family))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[connect fail"
                                << " local_family:" << m_family << " remote_family:" << addr->get_family() << "]";
    }
    if (timeout_ms != (uint64_t)-1)
    {
        FDEntity::ptr entity = SingletonFDManager::Instance()->get_fd(m_sock);
        if (TIGER_LIKELY(entity))
        {
            entity->set_connect_timeout(timeout_ms);
        }
    }
    if (::connect(m_sock, addr->get_addr(), addr->get_addr_len()))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[connect fail"
                                << " sock:" << m_sock << " errno:" << strerror(errno) << " addr:" << addr << "]";
        close();
        return false;
    }
    m_is_connected = true;
    init_local_address();
    init_remote_address();
    return true;
}

bool Socket::reconnect(const uint64_t timeout_ms)
{
    if (!m_remote_addr)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[reconnect fail remote address is nullptr]";
        return false;
    }
    m_local_addr.reset();
    return connect(m_remote_addr, timeout_ms);
}

bool Socket::bind(const Address::ptr addr)
{
    if (!is_valid())
    {
        new_sock();
        if (TIGER_UNLIKELY(!is_valid()))
        {
            TIGER_LOG_E(SYSTEM_LOG) << "[bind fail"
                                    << " sock:" << m_sock << "]";
            return false;
        }
    }
    if (TIGER_UNLIKELY(addr->get_family() != m_family))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[bind fail"
                                << " local_family:" << m_family << " remote_family:" << addr->get_family() << "]";
        return false;
    }
    if (::bind(m_sock, addr->get_addr(), addr->get_addr_len()))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[bind fail"
                                << " sock:" << m_sock << " errno:" << strerror(errno) << " addr:" << addr << "]";
        return false;
    }
    init_local_address();
    return true;
}

bool Socket::listen(int backlog)
{
    if (!is_valid())
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[listen fail"
                                << " sock:" << m_sock << "]";
        return false;
    }
    if (::listen(m_sock, backlog))
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[listen fail"
                                << " sock:" << m_sock << " errno:" << strerror(errno) << "]";
        return false;
    }
    return true;
}

Socket::ptr Socket::accept()
{
    auto socket = std::make_shared<Socket>(m_family, m_type, m_protocol);
    int sock = ::accept(m_sock, nullptr, nullptr);
    if (sock == -1)
    {
        if (m_is_connected)
        {
            TIGER_LOG_E(SYSTEM_LOG) << "[accept fail"
                                    << " sock:" << sock << " type:" << m_type << " protocol:" << m_protocol
                                    << " errno:" << strerror(errno) << "]";
        }
        return nullptr;
    }
    if (socket->init(sock))
    {
        return socket;
    }
    return nullptr;
}

bool Socket::close()
{
    if (!is_connected() && m_sock == -1)
    {
        return true;
    }
    m_is_connected = false;
    if (m_sock != -1)
    {
        ::close(m_sock);
        m_sock = -1;
    }
    return true;
}

int Socket::send(const void *buffer, size_t len, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        return ::send(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::send(const iovec *buffers, size_t len, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec *)buffers;
        msg.msg_iovlen = len;
        msg.msg_flags = flags;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::send_to(const void *buffer, size_t len, const Address::ptr to, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        return ::sendto(m_sock, buffer, len, flags, to->get_addr(), to->get_addr_len());
    }
    return -1;
}

int Socket::send_to(const iovec *buffers, size_t len, const Address::ptr to, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec *)buffers;
        msg.msg_iovlen = len;
        msg.msg_flags = flags;
        msg.msg_name = to->get_addr();
        msg.msg_namelen = to->get_addr_len();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void *buffer, size_t len, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        return ::recv(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::recv(iovec *buffers, size_t len, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = buffers;
        msg.msg_iovlen = len;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv_from(void *buffer, size_t len, Address::ptr from, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        socklen_t len = from->get_addr_len();
        return ::recvfrom(m_sock, buffer, len, flags, from->get_addr(), &len);
    }
    return -1;
}

int Socket::recv_from(iovec *buffers, size_t len, Address::ptr from, int flags)
{
    if (TIGER_LIKELY(is_connected()))
    {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec *)buffers;
        msg.msg_iovlen = len;
        msg.msg_name = from->get_addr();
        msg.msg_namelen = from->get_addr_len();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

bool Socket::cancel_read()
{
    return IOManager::GetThreadIOM()->cancel_event(m_sock, IOManager::EventStatus::READ);
}

bool Socket::cancel_write()
{
    return IOManager::GetThreadIOM()->cancel_event(m_sock, IOManager::EventStatus::WRITE);
}

bool Socket::cancel_accept()
{
    return IOManager::GetThreadIOM()->cancel_event(m_sock, IOManager::EventStatus::READ);
}

bool Socket::cancel_all()
{
    return IOManager::GetThreadIOM()->cancel_all_event(m_sock);
}

std::ostream &Socket::dump(std::ostream &os) const
{
    os << "[Socket sock:" << m_sock << " connected:" << m_is_connected << " family:" << m_family << " type:" << m_type
       << " protocol:" << m_protocol;
    if (m_local_addr)
    {
        os << " localAddr:" << m_local_addr;
    }
    if (m_remote_addr)
    {
        os << " remoteAddr:" << m_remote_addr;
    }
    os << "]";
    return os;
}

std::string Socket::to_string() const
{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const Socket::ptr socket)
{
    socket->dump(os);
    return os;
}

std::ostream &operator<<(std::ostream &os, const Socket &socket)
{
    socket.dump(os);
    return os;
}

SSLSocket::SSLSocket(int family, int type, int protocol) : Socket(family, type, protocol)
{
}

SSLSocket::~SSLSocket()
{
}

bool SSLSocket::init(int sock)
{
    bool v = Socket::init(sock);
    if (v)
    {
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sock);
        v = (SSL_accept(m_ssl.get()) == 1);
    }
    return v;
}

Socket::ptr SSLSocket::CreateTCP(tiger::Address::ptr address)
{
    return std::make_shared<SSLSocket>(address->get_family(), SOCK_STREAM, 0);
}

Socket::ptr SSLSocket::CreateTCPSocket()
{
    return std::make_shared<SSLSocket>(AF_INET, SOCK_STREAM, 0);
}

Socket::ptr SSLSocket::CreateTCPSocket6()
{
    return std::make_shared<SSLSocket>(AF_INET6, SOCK_STREAM, 0);
}

bool SSLSocket::connect(const Address::ptr addr, uint64_t timeout_ms)
{
    bool v = Socket::connect(addr, timeout_ms);
    if (v)
    {
        m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
        m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sock);
        v = (SSL_connect(m_ssl.get()) == 1);
    }
    return v;
}

bool SSLSocket::bind(const Address::ptr addr)
{
    return Socket::bind(addr);
}

bool SSLSocket::listen(int backlog)
{
    return Socket::listen(backlog);
}

Socket::ptr SSLSocket::accept()
{
    auto socket = std::make_shared<SSLSocket>(m_family, m_type, m_protocol);
    int sock = ::accept(m_sock, nullptr, nullptr);
    if (sock == -1)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[accept fail"
                                << " sock:" << m_sock << " type:" << m_type << " protocol:" << m_protocol
                                << " errno:" << strerror(errno) << "]";
        return nullptr;
    }
    socket->m_ctx = m_ctx;
    if (socket->init(sock))
    {
        return socket;
    }
    return nullptr;
}

bool SSLSocket::close()
{
    return Socket::close();
}

int SSLSocket::send(const void *buffer, size_t len, int flags)
{
    if (m_ssl)
        return SSL_write(m_ssl.get(), buffer, len);
    return -1;
}

int SSLSocket::send(const iovec *buffers, size_t len, int flags)
{
    if (!m_ssl)
        return -1;
    int total = 0;
    for (size_t i = 0; i < len; ++i)
    {
        int tmp = SSL_write(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
        if (tmp <= 0)
        {
            return tmp;
        }
        total += tmp;
        if (tmp != (int)buffers[i].iov_len)
        {
            break;
        }
    }
    return total;
}

int SSLSocket::send_to(const void *buffer, size_t len, const Address::ptr to, int flags)
{
    TIGER_ASSERT_WITH_INFO(false, "[SSL called send_to function]");
    return -1;
}

int SSLSocket::send_to(const iovec *buffers, size_t len, const Address::ptr to, int flags)
{
    TIGER_ASSERT_WITH_INFO(false, "[SSL called send_to function]");
    return -1;
}

int SSLSocket::recv(void *buffer, size_t len, int flags)
{
    if (m_ctx)
        return SSL_read(m_ssl.get(), buffer, len);
    return -1;
}

int SSLSocket::recv(iovec *buffers, size_t len, int flags)
{
    if (!m_ssl)
        return -1;
    int total = 0;
    for (size_t i = 0; i < len; ++i)
    {
        int tmp = SSL_read(m_ssl.get(), buffers[i].iov_base, buffers->iov_len);
        if (tmp <= 0)
        {
            return tmp;
        }
        total += tmp;
        if (tmp != (int)buffers[i].iov_len)
        {
            break;
        }
    }
    return total;
}

int SSLSocket::recv_from(void *buffer, size_t len, Address::ptr from, int flags)
{
    TIGER_ASSERT_WITH_INFO(false, "[SSL called send_to function]");
    return -1;
}

int SSLSocket::recv_from(iovec *buffers, size_t len, Address::ptr from, int flags)
{
    TIGER_ASSERT_WITH_INFO(false, "[SSL called send_to function]");
    return -1;
}

bool SSLSocket::load_certificates(const std::string &cert_file, const std::string &key_file)
{
    m_ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
    if (SSL_CTX_use_certificate_chain_file(m_ctx.get(), cert_file.c_str()) != 1)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[SSL_CTX_use_certificate_chain_file fail"
                                << " cert_file:" << cert_file << "]";
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(m_ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[SSL_CTX_use_PrivateKey_file fail"
                                << " key_file:" << key_file << "]";
        return false;
    }
    if (SSL_CTX_check_private_key(m_ctx.get()) != 1)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[SSL_CTX_check_private_key fail"
                                << " cert_file:" << cert_file << " key_file:" << key_file << "]";
        return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const SSLSocket::ptr socket)
{
    socket->dump(os);
    return os;
}

std::ostream &operator<<(std::ostream &os, const SSLSocket &socket)
{
    socket.dump(os);
    return os;
}

} // namespace tiger