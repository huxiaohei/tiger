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
    /********************************************************** Connection **********************************************************/
    bool PING();
    std::string ECHO(const std::string &msg);
    /********************************************************** Connection **********************************************************/

    /********************************************************** Key **********************************************************/
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
    /********************************************************** Key **********************************************************/

    /********************************************************** String **********************************************************/
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
    /********************************************************** String **********************************************************/

    /********************************************************** Hash **********************************************************/
    size_t HDEL(const std::string &hash, size_t n, ...);
    int HSET(const std::string &hash, const std::string &field, const std::string &val);
    bool HMSET(const std::string &hash, size_t n, ...);
    int HSETNX(const std::string &hash, const std::string &field, const std::string &val);
    int HINCRBY(const std::string &hash, const std::string &field, int incr_by_number);
    double HINCRBYFLOAT(const std::string &hash, const std::string &field, double incr_by_number);
    bool HEXISTS(const std::string &hash, const std::string &field);
    std::vector<std::string> HKEYS(const std::string &hash);
    size_t HLEN(const std::string &hash);
    std::vector<std::string> HVALS(const std::string &hash);
    std::vector<std::string> HGETALL(const std::string &hash);
    template <typename T>
    T HGET(const std::string &hash, const std::string &field) {
        std::string cmd = fmt::format("*3\r\n$4\r\nHGET\r\n${}\r\n{}\r\n${}\r\n{}\r\n", hash.size(), hash, field.size(), field);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> HMGET(const std::string &hash, size_t n, ...) {
        std::string cmd = fmt::format("*{}\r\n$5\r\nHMGET\r\n${}\r\n{}\r\n", n + 2, hash.size(), hash);
        va_list li;
        va_start(li, n);
        char *field = nullptr;
        for (size_t i = 0; i < n; ++i) {
            field = va_arg(li, char *);
            cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
        }
        va_end(li);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    /********************************************************** Hash **********************************************************/

    /********************************************************** List **********************************************************/
    template <typename T>
    T LPOP(const std::string &list) {
        std::string cmd = fmt::format("*2\r\n$4\r\nLPOP\r\n${}\r\n{}\r\n", list.size(), list);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    T RPOP(const std::string &list) {
        std::string cmd = fmt::format("*2\r\n$4\r\nRPOP\r\n${}\r\n{}\r\n", list.size(), list);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    std::vector<std::string> BLPOP(time_t timeout_ms, size_t n, ...);
    std::vector<std::string> BRPOP(time_t timeout_ms, size_t n, ...);
    template <typename T>
    T RPOPLPUSH(const std::string &list, const std::string &another_list) {
        std::string cmd = fmt::format("*3\r\n$9\r\nRPOPLPUSH\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, another_list.size(), another_list);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> BRPOPLPUSH(const std::string &list, const std::string &another_list, time_t timeout_ms) {
        time_t timeout = timeout_ms / 1000;
        std::string cmd = fmt::format("*4\r\n$10\r\nBRPOPLPUSH\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, another_list.size(), another_list, std::to_string(timeout).size(), timeout);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd, timeout_ms);
        return rst->get_data();
    }
    size_t LPUSH(const std::string &list, size_t n, ...);
    size_t RPUSH(const std::string &list, size_t n, ...);
    size_t LPUSHX(const std::string &list, size_t n, ...);
    size_t RPUSHX(const std::string &list, size_t n, ...);
    int64_t LINSERTBEFORE(const std::string &list, const std::string &exist, const std::string &val);
    int64_t LINSERTAFTER(const std::string &list, const std::string &exist, const std::string &val);
    bool LSET(const std::string &list, size_t idx, const std::string &val);
    template <typename T>
    T LINDEX(const std::string &list, size_t idx) {
        std::string cmd = fmt::format("*3\r\n$6\r\nLINDEX\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, std::to_string(idx).size(), idx);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> LRANGE(const std::string &list, int64_t start, int64_t end) {
        std::string cmd = fmt::format("*4\r\n$6\r\nLRANGE\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, std::to_string(start).size(), start, std::to_string(end).size(), end);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    size_t LREM(const std::string &list, int64_t count, const std::string &val);
    bool LTRIM(const std::string &list, int64_t start, int64_t end);
    size_t LLEN(const std::string &list);
    /********************************************************** List **********************************************************/

    /********************************************************** Set **********************************************************/
    size_t SADD(const std::string &set, size_t n, ...);
    size_t SCARD(const std::string &set);
    template <typename T>
    std::vector<T> SDIFF(size_t n, ...) {
        std::string cmd = fmt::format("*{}\r\n$5\r\nSDIFF\r\n", n + 1);
        va_list li;
        va_start(li, n);
        char *field = nullptr;
        for (size_t i = 0; i < n; ++i) {
            field = va_arg(li, char *);
            cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
        }
        va_end(li);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    size_t SDIFFSTORE(const std::string &set, size_t n, ...);
    template <typename T>
    std::vector<T> SINTER(size_t n, ...) {
        std::string cmd = fmt::format("*{}\r\n$6\r\nSINTER\r\n", n + 1);
        va_list li;
        va_start(li, n);
        char *field = nullptr;
        for (size_t i = 0; i < n; ++i) {
            field = va_arg(li, char *);
            cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
        }
        va_end(li);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    size_t SINTERSTORE(const std::string &set, size_t n, ...);
    template <typename T>
    std::vector<T> SUNION(size_t n, ...) {
        std::string cmd = fmt::format("*{}\r\n$6\r\nSUNION\r\n", n + 1);
        va_list li;
        va_start(li, n);
        char *field = nullptr;
        for (size_t i = 0; i < n; ++i) {
            field = va_arg(li, char *);
            cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
        }
        va_end(li);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    size_t SUNIONSTORE(const std::string &set, size_t n, ...);
    template <typename T>
    std::vector<T> SMEMBERS(const std::string &set) {
        std::string cmd = fmt::format("*2\r\n$8\r\nSMEMBERS\r\n${}\r\n{}\r\n", set.size(), set);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    bool SISMEMBER(const std::string &set, const std::string &member);
    bool SMOVE(const std::string &set, const std::string &other, const std::string &member);
    template <typename T>
    T SPOP(const std::string &set) {
        std::string cmd = fmt::format("*2\r\n$4\r\nSPOP\r\n${}\r\n{}\r\n", set.size(), set);
        auto rst = get_connection()->exec_cmd<RedisResultVal<T>>(cmd);
        return rst->get_data();
    }
    template <typename T>
    std::vector<T> SRANDMEMBER(const std::string &set, int64_t count) {
        std::string cmd = fmt::format("*3\r\n$11\r\nSRANDMEMBER\r\n${}\r\n{}\r\n${}\r\n{}\r\n", set.size(), set, std::to_string(count).size(), count);
        auto rst = get_connection()->exec_cmd<RedisResultVector<T>>(cmd);
        return rst->get_data();
    }
    size_t SREM(const std::string &set, size_t n, ...);
    /********************************************************** Set **********************************************************/
};

}  // namespace redis

}  // namespace tiger

#endif