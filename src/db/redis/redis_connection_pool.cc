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

bool RedisConnectionPool::SET(const std::string &key, const std::string &val) {
    auto cmd = fmt::format("*3\r\n$3\r\nSET\r\n${}\r\n{}\r\n${}\r\n{}\r\n", key.size(), key, val.size(), val);
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
}

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
    auto rst = get_connection()->exec_cmd<RedisResultVal<std::string>>(cmd);
    return rst->get_data() == "OK";
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

}  // namespace redis

}  // namespace tiger