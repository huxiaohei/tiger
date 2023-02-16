/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/10
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REDIS_CONNECTION_POOL_H__
#define __TIGER_REDIS_REDIS_CONNECTION_POOL_H__

#include "redis_connection.h"

namespace tiger {

namespace redis {

class RedisConnectionPool {
   private:
    IPAddress::ptr m_addr;
    std::string m_password;
    int32_t m_max_size;
    bool m_ssl;
    MutexLock m_mutex;
    std::list<RedisConnection *> m_conns;
    std::atomic<int32_t> m_total = {0};

   public:
    typedef std::shared_ptr<RedisConnectionPool> ptr;

    RedisConnectionPool(IPAddress::ptr addr, const std::string &password, int32_t max_size, bool ssl);
    virtual ~RedisConnectionPool();

   public:
    int32_t get_conn_total() { return m_total; }

   public:
    RedisConnection::ptr get_connection();
    void release_conn();

   public:
    static RedisConnectionPool::ptr Create(IPAddress::ptr addr, const std::string &password, int32_t max_size, bool ssl = false);
    static RedisConnectionPool::ptr Create(const std::string &host, int32_t port, const std::string &password, int32_t max_size, bool ssl = false);
    static void ReleaseConnPtr(RedisConnection *ptr, RedisConnectionPool *pool);

   public:
    bool SET(const std::string &key, const std::string &val);
    bool DEL(const std::string &key);
    std::string TYPE(const std::string &key);
    bool EXISTS(const std::string &key);
    bool MOVE(const std::string &key, const std::string &db);
    bool RENAME(const std::string &old_key, const std::string &new_key);
    bool RENAMENX(const std::string &old_key, const std::string &new_key);
    bool PEXPIREAT(const std::string &key, time_t ts_ms);
    bool EXPIREAT(const std::string &key, time_t ts_s);
    bool PEXPIRE(const std::string &key, time_t ms);
    bool EXPIRE(const std::string &key, time_t s);
    bool PERSIST(const std::string &key);
    time_t TTL(const std::string &key);
    time_t PTTL(const std::string &key);
    std::string RANDOMKEY();
    std::string DUMP(const std::string &key);
    std::vector<std::string> KEYS(const std::string &pattern);
};

}  // namespace redis

}  // namespace tiger

#endif