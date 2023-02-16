/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/06
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REDIS_CONNECTION_H__
#define __TIGER_REDIS_REDIS_CONNECTION_H__

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "../../streams/socket_stream.h"
#include "../../uri.h"
#include "redis.h"

namespace tiger {

namespace redis {

class RedisConnection : public SocketStream {
   private:
    IPAddress::ptr m_addr;
    std::string m_password;
    time_t m_last_rtime;
    RedisStatus m_status;

   protected:
    std::string pack_commond(const std::string &org_cmd);
    void send_commond(const std::string &cmd, RedisResult::ptr result, bool check_health = true);
    void read_response(RedisResult::ptr result);

   public:
    typedef std::shared_ptr<RedisConnection> ptr;

    RedisConnection(Socket::ptr socket, IPAddress::ptr addr, const std::string &pwd);
    ~RedisConnection();

   public:
    bool is_ok();
    bool ping(bool force = false);

    template <typename T>
    std::shared_ptr<T> exec_cmd(const std::string &cmd, bool check_health = true) {
        auto rst = std::make_shared<T>();
        send_commond(cmd, rst, check_health);
        if (rst->get_status() == RedisStatus::CONNECT_FAIL) {
            throw std::runtime_error(fmt::format("[Redis exec [{}] => [{} {}]", cmd, rst->get_status(), rst->get_err_desc()));
            return rst;
        }
        read_response(rst);
        if (rst->get_status() != RedisStatus::OK) {
            TIGER_LOG_W(SYSTEM_LOG) << "[Redis exec [" << cmd << "]"
                                    << " => [" << rst->get_status()
                                    << " " << rst->get_err_desc() << "]";
            throw std::runtime_error(fmt::format("[Redis exec [{}] => [{} {}]", cmd, rst->get_status(), rst->get_err_desc()));
        }
        return rst;
    }

   public:
    static RedisConnection::ptr Create(IPAddress::ptr addr, const std::string &pwd, bool ssl);
    static RedisConnection::ptr Create(const std::string &host, int32_t port, const std::string &pwd, bool ssl = false);
};

}  // namespace redis
}  // namespace tiger

#endif