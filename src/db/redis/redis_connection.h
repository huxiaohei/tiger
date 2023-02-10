/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/06
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REDIS_CONNECTION_H__
#define __TIGER_REDIS_REDIS_CONNECTION_H__

#include "../../streams/socket_stream.h"
#include "../../uri.h"
#include "redis.h"

namespace tiger {

namespace redis {

class RedisConnection : public SocketStream {
   public:
    typedef std::shared_ptr<RedisConnection> ptr;

    RedisConnection(Socket::ptr socket, IPAddress::ptr addr, const std::string &pwd);
    ~RedisConnection();

   public:
    bool ping(bool force = false);
    template <typename T>
    std::shared_ptr<T> exec_cmd(const std::string &cmd, bool check_health = true);
    template <typename T>
    void read_response(std::shared_ptr<T> result);

   public:
    static RedisConnection::ptr CreateRedisConnection(IPAddress::ptr addr, const std::string &pwd, bool ssl);
    static RedisConnection::ptr CreateRedisConnection(const std::string &host, int32_t port, const std::string &pwd, bool ssl = false);

   private:
    std::string pack_commond(const std::string &org_cmd);
    template <typename T>
    void send_commond(const std::string &cmd, std::shared_ptr<T> result, bool check_health = true);

   private:
    IPAddress::ptr m_addr;
    std::string m_password;
    time_t m_last_rtime;
    RedisStatus m_status;
};

}  // namespace redis
}  // namespace tiger

#endif