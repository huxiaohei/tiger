/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "socket_stream.h"

namespace tiger
{

SocketStream::SocketStream(Socket::ptr socket, bool owner) : m_socket(socket), m_owner(owner)
{
}

SocketStream::~SocketStream()
{
    if (m_owner && m_socket)
    {
        m_socket->close();
    }
}

bool SocketStream::is_connected() const
{
    return m_socket && m_socket->is_connected();
}

Address::ptr SocketStream::get_remote_address()
{
    if (!m_socket)
        return nullptr;
    return m_socket->get_remote_addr();
}

Address::ptr SocketStream::get_local_address()
{
    if (!m_socket)
        return nullptr;
    return m_socket->get_local_addr();
}

int SocketStream::read(void *buffer, size_t len)
{
    if (!is_connected())
        return -1;
    return m_socket->recv(buffer, len);
}

int SocketStream::read(ByteArray::ptr ba, size_t len)
{
    if (!is_connected())
        return -1;
    std::vector<iovec> iovs;
    ba->get_enable_write_buffers(iovs, len);
    int rt = m_socket->recv(&iovs[0], iovs.size());
    if (rt > 0)
    {
        ba->set_position(ba->get_position() + rt);
    }
    return rt;
}

int SocketStream::write(const void *buffer, size_t len)
{
    if (!is_connected())
        return -1;
    return m_socket->send(buffer, len);
}

int SocketStream::write(ByteArray::ptr ba, size_t len)
{
    if (!is_connected())
        return -1;
    std::vector<iovec> iovs;
    ba->get_enable_read_buffers(iovs, len);
    int rt = m_socket->send(&iovs[0], iovs.size());
    if (rt > 0)
    {
        ba->set_position(ba->get_position() + rt);
    }
    return rt;
}

void SocketStream::close()
{
    if (m_socket)
        m_socket->close();
}

} // namespace tiger