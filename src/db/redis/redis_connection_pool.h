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
    bool PING();

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

    bool SET(const std::string &key, const std::string &val);
    bool SETNX(const std::string &key, const std::string &val);
    bool MSET(size_t n, ...);
    bool MSET(const std::vector<std::pair<std::string, std::string>> &keys);
    bool MSETNX(size_t n, ...);
    bool MSETNX(const std::vector<std::pair<std::string, std::string>> &keys);
    bool SETEX(const std::string &key, time_t s, const std::string &val);
    bool PSETEX(const std::string &key, time_t ms, const std::string &val);
    int SETBIT(const std::string &key, int offset, int val);
    int SETRANGE(const std::string &key, int offset, const std::string &val);
    size_t APPEND(const std::string &key, const std::string &val);
    long int INCR(const std::string &key);
    long int INCRBY(const std::string &key, long int val);
    double INCRBYFLOAT(const std::string &key, double val);
    long int DECR(const std::string &key);
    long int DECRBY(const std::string &key, long int val);
    template <typename T>
    T GETSET(const std::string &key, const std::string &val) {
        auto cmd = fmt::format("*3\r\n$6\r\nGETSET\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, val.size(), val);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    T GET(const std::string &key) {
        auto cmd = fmt::format("*2\r\n$3\r\nGET\r\n${}\r\n{}\r\n", key.size(), key);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> MGET(size_t n, ...) {
        std::string cmd = fmt::format("*{}\r\n$4\r\nMGET\r\n", n + 1);
        va_list li;
        va_start(li, n);
        char *key = nullptr;
        for (size_t i = 0; i < n; ++i) {
            key = va_arg(li, char *);
            cmd += fmt::format("${}\r\n{}\r\n", strlen(key), key);
        }
        va_end(li);
        auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> MGET(const std::vector<std::string> &keys) {
        std::string cmd = fmt::format("*{}\r\n$4\r\nMGET\r\n", keys.size() + 1);
        for (auto &key : keys) {
            cmd += fmt::format("${}\r\n{}\r\n", key.size(), key);
        }
        auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
        return rst->get_data();
    }
    int GETBIT(const std::string &key, int offset);
    std::string GETRANGE(const std::string &key, int start, int end);
    int STRLEN(const std::string &key);
};

}  // namespace redis

}  // namespace tiger

#endif