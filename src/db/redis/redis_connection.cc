/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "redis_connection.h"

#include "../../config.h"

namespace tiger {

namespace redis {

static ConfigVar<size_t>::ptr g_redis_connect_timeout = Config::Lookup<size_t>("tiger.redis.g_redis_connect_timeout", 6 * 1000, "redis connect timeout");
static ConfigVar<size_t>::ptr g_redis_send_timeout = Config::Lookup<size_t>("tiger.redis.g_redis_send_timeout", 6 * 1000, "redis send timeout");
static ConfigVar<size_t>::ptr g_redis_recv_timeout = Config::Lookup<size_t>("tiger.redis.g_redis_recv_timeout", 6 * 1000, "redis recv timeout");
static ConfigVar<int64_t>::ptr g_redis_ping = Config::Lookup<int64_t>("tiger.redis.ping", 6, "redis connection ping");
static ConfigVar<uint64_t>::ptr g_redis_response_buffer_size = Config::Lookup<uint64_t>("tiger.redis.responseBufferSize", 4 * 1024, "redis response buffer size");

RedisConnection::RedisConnection(Socket::ptr socket, IPAddress::ptr addr, const std::string &pwd)
    : SocketStream(socket, true),
      m_addr(addr),
      m_password(pwd),
      m_last_rtime(-1),
      m_status(RedisStatus::INVALID) {
    if (!socket->connect(addr, g_redis_connect_timeout->val())) {
        m_status = RedisStatus::CONNECT_FAIL;
        TIGER_LOG_E(SYSTEM_LOG) << "[redis connect fail addr:" << *addr << "]";
        return;
    }
    socket->set_recv_timeout(g_redis_recv_timeout->val());
    socket->set_send_timeout(g_redis_send_timeout->val());
    if (!m_password.empty()) {
        RedisResultStr::ptr rst = exec_cmd_format<RedisResultStr>(false, "AUTH", "%s", m_password.c_str());
        if (rst->is_ok()) {
            m_status = RedisStatus::OK;
        } else {
            m_status = RedisStatus::AUTH_FAIL;
        }
    } else {
        m_status = RedisStatus::OK;
    }
}

RedisConnection::~RedisConnection() {
}

RedisConnection::ptr RedisConnection::CreateRedisConnection(
    IPAddress::ptr addr,
    const std::string &pwd,
    bool ssl) {
    auto socket = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    return std::make_shared<RedisConnection>(socket, addr, pwd);
}

RedisConnection::ptr RedisConnection::CreateRedisConnection(
    const std::string &host,
    int32_t port,
    const std::string &pwd,
    bool ssl) {
    auto addr = IPAddress::LookupAny(host);
    addr->set_port(port);
    return CreateRedisConnection(addr, pwd, ssl);
}

bool RedisConnection::ping(bool force) {
    auto now = Second();
    if (force || (now - m_last_rtime > g_redis_ping->val())) {
        auto rst = exec_cmd<RedisResultStr>("PING", false);
    }
    return true;
}

template <typename T>
std::shared_ptr<T> RedisConnection::exec_cmd_format(bool check_health, const std::string &cmd, const char *fmt, ...) {
    std::stringstream ss;
    ss << cmd << " ";
    va_list al;
    va_start(al, fmt);
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        ss << std::string(buf, len);
        free(buf);
    }
    va_end(al);
    return exec_cmd<T>(ss.str(), check_health);
}

template <typename T>
std::shared_ptr<T> RedisConnection::exec_cmd(const std::string &cmd, bool check_health) {
    auto rst = std::make_shared<T>();
    send_commond(cmd, rst, check_health);
    if (rst->get_status() == RedisStatus::CONNECT_FAIL) {
        return rst;
    }
    read_response(rst);
    return rst;
}

std::string RedisConnection::pack_commond(const std::string &org_cmd) {
    std::stringstream ss;
    ss << org_cmd;
    std::vector<std::string> segments;
    std::string segment;
    while (getline(ss, segment, ' ')) {
        segments.push_back(segment);
    }
    ss.clear();
    ss.str("");
    ss << "*" << segments.size() << CRLF;
    for (auto &it : segments) {
        ss << "$" << it.size() << CRLF << it << CRLF;
    }
    return ss.str();
}

template <typename T>
void RedisConnection::send_commond(const std::string &cmd, std::shared_ptr<T> result, bool check_health) {
    if (check_health && !ping()) {
        result->set_status(RedisStatus::CONNECT_FAIL);
        return;
    }
    auto package = pack_commond(cmd);
    write_fixed_size(package.c_str(), package.size());
}

template <typename T>
void RedisConnection::read_response(std::shared_ptr<T> result) {
    uint64_t buffer_size = g_redis_response_buffer_size->val();
    std::shared_ptr<char> buffer(new char[buffer_size + 1], [](char *ptr) {
        delete[] ptr;
    });
    char *data = buffer.get();
    uint64_t offset = 0;
    do {
        int len = read(data + offset, buffer_size - offset);
        if (len <= 0) {
            result->set_status(RedisStatus::CONNECT_FAIL);
            m_status = RedisStatus::INVALID;
            close();
            return;
        }
        data[len] = '\0';
        offset += len;
        if (offset == buffer_size) {
            result->set_status(RedisStatus::READ_OVERFLOW);
            m_status = RedisStatus::INVALID;
            close();
            return;
        }
        result->parse(data, len);
        if (result->parse_finished()) {
            break;
        }
        if (result->is_parse_error()) {
            m_status = RedisStatus::INVALID;
            close();
            return;
        }
    } while (true);
}

}  // namespace redis

}  // namespace tiger
