/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_SOCKET_STREAM_H__
#define __TIGER_SOCKET_STREAM_H__

#include "../iomanager.h"
#include "../mutex.h"
#include "../socket.h"
#include "stream.h"

namespace tiger {

class SocketStream : public Stream {
   private:
    Socket::ptr m_socket;
    bool m_owner;

   public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr socket, bool owner = true);
    virtual ~SocketStream();

   public:
    Socket::ptr get_socket() const { return m_socket; }
    bool is_connected() const;
    Address::ptr get_remote_address();
    Address::ptr get_local_address();

   public:
    virtual int read(void *buffer, size_t len) override;
    virtual int read(ByteArray::ptr ba, size_t len) override;
    virtual int write(const void *buffer, size_t len) override;
    virtual int write(ByteArray::ptr ba, size_t len) override;
    virtual void close() override;
};
}  // namespace tiger
#endif