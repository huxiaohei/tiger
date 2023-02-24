/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/10
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "redis_connection_pool.h"

#include <unistd.h>

namespace tiger {

namespace redis {

RedisConnectionPool::ptr RedisConnectionPool::Create(IPAddress::ptr addr, const std::string &password, int32_t max_size, bool ssl) {
    RedisConnectionPool::ptr pool = std::make_shared<RedisConnectionPool>(addr, password, max_size, ssl);
    return pool;
}

RedisConnectionPool::ptr RedisConnectionPool::Create(const std::string &host, int32_t port, const std::string &password, int32_t max_size, bool ssl) {
    auto addr = IPAddress::LookupAny(host);
    addr->set_port(port);
    return Create(addr, password, max_size, ssl);
}

void RedisConnectionPool::ReleaseConnPtr(RedisConnection *conn, RedisConnectionPool *pool) {
    if (!conn->is_ok()) {
        delete conn;
        --pool->m_total;
        return;
    }
    MutexLock::Lock lock(pool->m_mutex);
    conn->reset_timeout();
    pool->m_conns.push_back(conn);
}

RedisConnectionPool::RedisConnectionPool(IPAddress::ptr addr, const std::string &password, int32_t max_size, bool ssl)
    : m_addr(addr), m_password(password), m_max_size(max_size), m_ssl(ssl) {
}

RedisConnectionPool::~RedisConnectionPool() {
    MutexLock::Lock lock(m_mutex);
    for (auto &it : m_conns) {
        delete it;
        --m_total;
    }
    TIGER_LOG_I(SYSTEM_LOG) << "[Redis Pool conn count is " << m_total << "]";
}

RedisConnection::ptr RedisConnectionPool::get_connection() {
    RedisConnection *conn = nullptr;
    do {
        std::vector<RedisConnection *> invalids;
        {
            MutexLock::Lock lock(m_mutex);
            while (!m_conns.empty()) {
                RedisConnection *tmp = *m_conns.begin();
                m_conns.pop_front();
                if (!tmp->is_ok()) {
                    invalids.push_back(tmp);
                    continue;
                }
                conn = tmp;
                break;
            }
        }
        for (auto it : invalids) {
            delete it;
        }
        m_total -= invalids.size();
        if (conn || m_max_size > m_total) break;
        usleep(2);
    } while (true);
    if (!conn) {
        auto socket = m_ssl ? SSLSocket::CreateTCP(m_addr) : Socket::CreateTCP(m_addr);
        conn = new RedisConnection(socket, m_addr, m_password);
        ++m_total;
    }
    return RedisConnection::ptr(conn, std::bind(&RedisConnectionPool::ReleaseConnPtr,
                                                std::placeholders::_1,
                                                this));
}

bool RedisConnectionPool::PING() {
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>("PING");
    return rst->get_data() == "PONG";
}

/********************************************************** Key **********************************************************/
bool RedisConnectionPool::DEL(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$3\r\nDEL\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

std::string RedisConnectionPool::TYPE(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$4\r\nTYPE\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::EXISTS(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$6\r\nEXISTS\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::MOVE(const std::string &key, const std::string &db) {
    auto cmd = fmt::format("*3\r\n$4\r\nMOVE\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, db.size(), db);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::RENAME(const std::string &old_key, const std::string &new_key) {
    auto cmd = fmt::format("*3\r\n$6\r\nRENAME\r\n${}\r\n{}\r\n${}\r\n{}\r\n", old_key.size(), old_key, new_key.size(), new_key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

bool RedisConnectionPool::RENAMENX(const std::string &old_key, const std::string &new_key) {
    auto cmd = fmt::format("*3\r\n$8\r\nRENAMENX\r\n${}\r\n{}\r\n${}\r\n{}\r\n", old_key.size(), old_key, new_key.size(), new_key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::PEXPIREAT(const std::string &key, time_t ts_ms) {
    auto cmd = fmt::format("*3\r\n$9\r\nPEXPIREAT\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(ts_ms).size(), ts_ms);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::EXPIREAT(const std::string &key, time_t ts_s) {
    auto cmd = fmt::format("*3\r\n$8\r\nEXPIREAT\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(ts_s).size(), ts_s);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::PEXPIRE(const std::string &key, time_t ms) {
    auto cmd = fmt::format("*3\r\n$7\r\nPEXPIRE\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(ms).size(), ms);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::EXPIRE(const std::string &key, time_t s) {
    auto cmd = fmt::format("*3\r\n$6\r\nEXPIRE\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(s).size(), s);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::PERSIST(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$7\r\nPERSIST\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

time_t RedisConnectionPool::TTL(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$3\r\nTTL\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<time_t>>(cmd);
    return rst->get_data();
}

time_t RedisConnectionPool::PTTL(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$4\r\nPTTL\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<time_t>>(cmd);
    return rst->get_data();
}

std::string RedisConnectionPool::RANDOMKEY() {
    auto cmd = fmt::format("RANDOMKEY");
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data();
}

std::string RedisConnectionPool::DUMP(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$4\r\nDUMP\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data();
}

std::vector<std::string> RedisConnectionPool::KEYS(const std::string &pattern) {
    auto cmd = fmt::format("*2\r\n$4\r\nKEYS\r\n${}\r\n{}\r\n", pattern.size(), pattern);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
    return rst->get_data();
}
/********************************************************** Key **********************************************************/

/********************************************************** String **********************************************************/
bool RedisConnectionPool::SET(const std::string &key, const std::string &val) {
    auto cmd = fmt::format("*3\r\n$3\r\nSET\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

bool RedisConnectionPool::SETNX(const std::string &key, const std::string &val) {
    auto cmd = fmt::format("*3\r\n$5\r\nSETNX\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::MSET(size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$4\r\nMSET\r\n", n * 2 + 1);
    va_list li;
    va_start(li, n);
    for (size_t i = 0; i < n; ++i) {
        char *key = nullptr;
        key = va_arg(li, char *);
        char *value = nullptr;
        value = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n${}\r\n{}\r\n", strlen(key), key, strlen(value), value);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

bool RedisConnectionPool::MSET(const std::vector<std::pair<std::string, std::string>> &keys) {
    std::string cmd = fmt::format("*{}\r\n$4\r\nMSET\r\n", keys.size() * 2 + 1);
    for (auto &it : keys) {
        cmd += fmt::format("${}\r\n{}\r\n${}\r\n{}\r\n", it.first.size(), it.first, it.second.size(), it.second);
    }
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

bool RedisConnectionPool::MSETNX(size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$6\r\nMSETNX\r\n", n * 2 + 1);
    va_list li;
    va_start(li, n);
    for (size_t i = 0; i < n; ++i) {
        char *key = nullptr;
        key = va_arg(li, char *);
        char *value = nullptr;
        value = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n${}\r\n{}\r\n", strlen(key), key, strlen(value), value);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::MSETNX(const std::vector<std::pair<std::string, std::string>> &keys) {
    std::string cmd = fmt::format("*{}\r\n$6\r\nMSETNX\r\n", keys.size() * 2 + 1);
    for (auto &it : keys) {
        cmd += fmt::format("${}\r\n{}\r\n${}\r\n{}\r\n", it.first.size(), it.first, it.second.size(), it.second);
    }
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::SETEX(const std::string &key, time_t s, const std::string &val) {
    auto cmd = fmt::format("*4\r\n$5\r\nSETEX\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(s).size(), s, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::PSETEX(const std::string &key, time_t ms, const std::string &val) {
    auto cmd = fmt::format("*4\r\n$6\r\nPSETEX\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(ms).size(), ms, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::SETBIT(const std::string &key, int offset, int val) {
    auto cmd = fmt::format("*4\r\n$6\r\nSETBIT\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(offset).size(), offset, std::to_string(val).size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::SETRANGE(const std::string &key, int offset, const std::string &val) {
    auto cmd = fmt::format("*4\r\n$8\r\nSETRANGE\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(offset).size(), offset, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::APPEND(const std::string &key, const std::string &val) {
    auto cmd = fmt::format("*3\r\n$6\r\nAPPEND\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

long int RedisConnectionPool::INCR(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$4\r\nINCR\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<long int>>(cmd);
    return rst->get_data();
}

long int RedisConnectionPool::INCRBY(const std::string &key, long int val) {
    auto cmd = fmt::format("*3\r\n$6\r\nINCRBY\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(val).size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<long int>>(cmd);
    return rst->get_data();
}

double RedisConnectionPool::INCRBYFLOAT(const std::string &key, double val) {
    auto cmd = fmt::format("*3\r\n$11\r\nINCRBYFLOAT\r\n${}\r\n{}\r\n${}\r\n{:.6f}\r\n", key.size(), key, std::to_string(val).size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<double>>(cmd);
    return rst->get_data();
}

long int RedisConnectionPool::DECR(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$4\r\nDECR\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<long int>>(cmd);
    return rst->get_data();
}

long int RedisConnectionPool::DECRBY(const std::string &key, long int val) {
    auto cmd = fmt::format("*3\r\n$6\r\nDECRBY\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(val).size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<long int>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::GETBIT(const std::string &key, int offset) {
    auto cmd = fmt::format("*3\r\n$6\r\nGETBIT\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(offset).size(), offset);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

std::string RedisConnectionPool::GETRANGE(const std::string &key, int start, int end) {
    auto cmd = fmt::format("*4\r\n$8\r\nGETRANGE\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, std::to_string(start).size(), start, std::to_string(end).size(), end);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::STRLEN(const std::string &key) {
    auto cmd = fmt::format("*2\r\n$6\r\nSTRLEN\r\n${}\r\n{}\r\n", key.size(), key);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}
/********************************************************** String **********************************************************/

/********************************************************** Hash **********************************************************/
size_t RedisConnectionPool::HDEL(const std::string &hash, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$4\r\nHDEL\r\n${}\r\n{}\r\n", n + 2, hash.size(), hash);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::HSET(const std::string &hash, const std::string &field, const std::string &val) {
    std::string cmd = fmt::format("*4\r\n$4\r\nHSET\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", hash.size(), hash, field.size(), field, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::HMSET(const std::string &hash, size_t n, ...) {
    n *= 2;
    std::string cmd = fmt::format("*{}\r\n$5\r\nHMSET\r\n${}\r\n{}\r\n", n + 2, hash.size(), hash);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

int RedisConnectionPool::HSETNX(const std::string &hash, const std::string &field, const std::string &val) {
    std::string cmd = fmt::format("*4\r\n$6\r\nHSETNX\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", hash.size(), hash, field.size(), field, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

int RedisConnectionPool::HINCRBY(const std::string &hash, const std::string &field, int incr_by_number) {
    std::string cmd = fmt::format("*4\r\n$7\r\nHINCRBY\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", hash.size(), hash, field.size(), field, std::to_string(incr_by_number), incr_by_number);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int>>(cmd);
    return rst->get_data();
}

double RedisConnectionPool::HINCRBYFLOAT(const std::string &hash, const std::string &field, double incr_by_number) {
    std::string cmd = fmt::format("*4\r\n$12\r\nHINCRBYFLOAT\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{:.6f}\r\n", hash.size(), hash, field.size(), field, std::to_string(incr_by_number).size(), incr_by_number);
    auto rst = get_connection()->exec_cmd<RedisResultVal<double>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::HEXISTS(const std::string &hash, const std::string &field) {
    std::string cmd = fmt::format("*3\r\n$7\r\nHEXISTS\r\n${}\r\n{}\r\n${}\r\n{}\r\n", hash.size(), hash, field.size(), field);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

std::vector<std::string> RedisConnectionPool::HKEYS(const std::string &hash) {
    std::string cmd = fmt::format("*2\r\n$5\r\nHKEYS\r\n${}\r\n{}\r\n", hash.size(), hash);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::HLEN(const std::string &hash) {
    std::string cmd = fmt::format("*2\r\n$4\r\nHLEN\r\n${}\r\n{}\r\n", hash.size(), hash);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

std::vector<std::string> RedisConnectionPool::HVALS(const std::string &hash) {
    std::string cmd = fmt::format("*2\r\n$5\r\nHVALS\r\n${}\r\n{}\r\n", hash.size(), hash);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
    return rst->get_data();
}

std::vector<std::string> RedisConnectionPool::HGETALL(const std::string &hash) {
    std::string cmd = fmt::format("*2\r\n$7\r\nHGETALL\r\n${}\r\n{}\r\n", hash.size(), hash);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd);
    return rst->get_data();
}
/********************************************************** Hash **********************************************************/

/********************************************************** List **********************************************************/
std::vector<std::string> RedisConnectionPool::BLPOP(time_t timeout_ms, size_t n, ...) {
    time_t timeout = timeout_ms / 1000;
    std::string cmd = fmt::format("*{}\r\n$5\r\nBLPOP\r\n", n + 2);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    cmd += fmt::format("${}\r\n{}\r\n", std::to_string(timeout).size(), timeout);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd, timeout_ms);
    return rst->get_data();
}

std::vector<std::string> RedisConnectionPool::BRPOP(time_t timeout_ms, size_t n, ...) {
    time_t timeout = timeout_ms / 1000;
    std::string cmd = fmt::format("*{}\r\n$5\r\nBRPOP\r\n", n + 2);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    cmd += fmt::format("${}\r\n{}\r\n", std::to_string(timeout).size(), timeout);
    auto rst = get_connection()->exec_cmd<RedisResultVector<std::string>>(cmd, timeout_ms);
    return rst->get_data();
}

size_t RedisConnectionPool::LPUSH(const std::string &list, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$5\r\nLPUSH\r\n${}\r\n{}\r\n", n + 2, list.size(), list);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::RPUSH(const std::string &list, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$5\r\nRPUSH\r\n${}\r\n{}\r\n", n + 2, list.size(), list);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::LPUSHX(const std::string &list, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$6\r\nLPUSHX\r\n${}\r\n{}\r\n", n + 2, list.size(), list);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::RPUSHX(const std::string &list, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$6\r\nRPUSHX\r\n${}\r\n{}\r\n", n + 2, list.size(), list);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

int64_t RedisConnectionPool::LINSERTBEFORE(const std::string &list, const std::string &exist, const std::string &val) {
    std::string cmd = fmt::format("*5\r\n$7\r\nLINSERT\r\n${}\r\n{}\r\n$6\r\nBEFORE\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, exist.size(), exist, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int64_t>>(cmd);
    return rst->get_data();
}

int64_t RedisConnectionPool::LINSERTAFTER(const std::string &list, const std::string &exist, const std::string &val) {
    std::string cmd = fmt::format("*5\r\n$7\r\nLINSERT\r\n${}\r\n{}\r\n$5\r\nAFTER\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, exist.size(), exist, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<int64_t>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::LSET(const std::string &list, size_t idx, const std::string &val) {
    std::string cmd = fmt::format("*4\r\n$4\r\nLSET\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, std::to_string(idx).size(), idx, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

size_t RedisConnectionPool::LREM(const std::string &list, int64_t count, const std::string &val) {
    std::string cmd = fmt::format("*4\r\n$4\r\nLREM\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, std::to_string(count).size(), count, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::LTRIM(const std::string &list, int64_t start, int64_t end) {
    std::string cmd = fmt::format("*4\r\n$5\r\nLTRIM\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", list.size(), list, std::to_string(start).size(), start, std::to_string(end).size(), end);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

size_t RedisConnectionPool::LLEN(const std::string &list) {
    std::string cmd = fmt::format("*2\r\n$4\r\nLLEN\r\n${}\r\n{}\r\n", list.size(), list);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}
/********************************************************** List **********************************************************/

/********************************************************** Set **********************************************************/
size_t RedisConnectionPool::SADD(const std::string &set, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$4\r\nSADD\r\n${}\r\n{}\r\n", n + 2, set.size(), set);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::SCARD(const std::string &set) {
    std::string cmd = fmt::format("*2\r\n$5\r\nSCARD\r\n${}\r\n{}\r\n", set.size(), set);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::SDIFFSTORE(const std::string &set, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$10\r\nSDIFFSTORE\r\n${}\r\n{}\r\n", n + 2, set.size(), set);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::SINTERSTORE(const std::string &set, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$11\r\nSINTERSTORE\r\n${}\r\n{}\r\n", n + 2, set.size(), set);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::SUNIONSTORE(const std::string &set, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$11\r\nSUNIONSTORE\r\n${}\r\n{}\r\n", n + 2, set.size(), set);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::SISMEMBER(const std::string &set, const std::string &member) {
    std::string cmd = fmt::format("*3\r\n$9\r\nSISMEMBER\r\n${}\r\n{}\r\n${}\r\n{}\r\n", set.size(), set, member.size(), member);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

bool RedisConnectionPool::SMOVE(const std::string &set, const std::string &other, const std::string &member) {
    std::string cmd = fmt::format("*4\r\n$5\r\nSMOVE\r\n${}\r\n{}\r\n${}\r\n{}\r\n${}\r\n{}\r\n", set.size(), set, other.size(), other, member.size(), member);
    auto rst = get_connection()->exec_cmd<RedisResultVal<bool>>(cmd);
    return rst->get_data();
}

size_t RedisConnectionPool::SREM(const std::string &set, size_t n, ...) {
    std::string cmd = fmt::format("*{}\r\n$4\r\nSREM\r\n${}\r\n{}\r\n", n + 2, set.size(), set);
    va_list li;
    va_start(li, n);
    char *field = nullptr;
    for (size_t i = 0; i < n; ++i) {
        field = va_arg(li, char *);
        cmd += fmt::format("${}\r\n{}\r\n", strlen(field), field);
    }
    va_end(li);
    auto rst = get_connection()->exec_cmd<RedisResultVal<size_t>>(cmd);
    return rst->get_data();
}
/********************************************************** Set **********************************************************/

}  // namespace redis

}  // namespace tiger