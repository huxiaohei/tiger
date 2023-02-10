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

}  // namespace redis

}  // namespace tiger