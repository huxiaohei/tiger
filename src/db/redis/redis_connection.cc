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
static ConfigVar<uint64_t>::ptr g_redis_response_buffer_size = Config::Lookup<uint64_t>("tiger.redis.responseBufferSize", 1024 * 1024, "redis response buffer size");

RedisConnection::RedisConnection(Socket::ptr socket, IPAddress::ptr addr, const std::string &pwd)
    : SocketStream(socket, true),
      m_addr(addr),
      m_password(pwd),
      m_last_rtime(-1),
      m_status(RedisStatus::INVALID),
      m_timeout(0) {
    if (!socket->connect(addr, g_redis_connect_timeout->val())) {
        m_status = RedisStatus::CONNECT_FAIL;
        TIGER_LOG_E(SYSTEM_LOG) << "[Redis connect fail [addr:" << *addr << "]]";
        return;
    }
    socket->set_recv_timeout(g_redis_recv_timeout->val());
    socket->set_send_timeout(g_redis_send_timeout->val());
    if (!m_password.empty()) {
        auto rst = exec_cmd<RedisResultVal<std::string>>("AUTH " + m_password, 0, false);
        if (rst->is_ok()) {
            m_status = RedisStatus::OK;
        } else {
            TIGER_LOG_W(TEST_LOG) << "[redis auth fail addr:" << *addr << " status:" << rst->get_status() << " reason:" << rst->get_data() << "]";
            socket->close();
            m_status = RedisStatus::AUTH_FAIL;
        }
    } else {
        m_status = RedisStatus::OK;
    }
}

RedisConnection::~RedisConnection() {
    close();
}

RedisConnection::ptr RedisConnection::Create(
    IPAddress::ptr addr,
    const std::string &pwd,
    bool ssl) {
    auto socket = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    return std::make_shared<RedisConnection>(socket, addr, pwd);
}

RedisConnection::ptr RedisConnection::Create(
    const std::string &host,
    int32_t port,
    const std::string &pwd,
    bool ssl) {
    auto addr = IPAddress::LookupAny(host);
    addr->set_port(port);
    return Create(addr, pwd, ssl);
}

bool RedisConnection::is_ok() {
    return m_status == RedisStatus::OK;
}

bool RedisConnection::ping(bool force) {
    auto now = Second();
    if (TIGER_UNLIKELY(m_status != RedisStatus::OK)) return false;
    if (force || (now - m_last_rtime > g_redis_ping->val())) {
        auto rst = exec_cmd<RedisResultVal<std::string>>(TIGER_REDIS_CMD_PING, 0, false);
        if (TIGER_UNLIKELY(rst->get_data() != "PONG")) {
            m_status = RedisStatus::CONNECT_FAIL;
            return false;
        }
    }
    return true;
}

void RedisConnection::reset_timeout() {
    if (m_timeout > 0) {
        get_socket()->set_recv_timeout(g_redis_recv_timeout->val());
        get_socket()->set_send_timeout(g_redis_send_timeout->val());
    }
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
    ss << "*" << segments.size() << TIGER_REDIS_CRLF;
    for (auto &it : segments) {
        ss << "$" << it.size() << TIGER_REDIS_CRLF << it << TIGER_REDIS_CRLF;
    }
    return ss.str();
}

void RedisConnection::send_commond(const std::string &cmd, RedisResult::ptr result, bool check_health) {
    if (check_health && !ping()) {
        result->set_status(RedisStatus::CONNECT_FAIL);
        result->set_err_desc("Err Redis connect fail");
        return;
    }
    if (cmd[0] == '*') {
        write_fixed_size(cmd.c_str(), cmd.size());
    } else {
        auto package = pack_commond(cmd);
        write_fixed_size(package.c_str(), package.size());
    }
}

void RedisConnection::read_response(RedisResult::ptr result) {
    uint64_t buffer_size = g_redis_response_buffer_size->val();
    std::shared_ptr<char> buffer(new char[buffer_size + 1], [](char *ptr) {
        delete[] ptr;
    });
    char *data = buffer.get();
    uint64_t offset = 0;
    do {
        int len = read(data + offset, buffer_size - offset);
        if (len < 0) {
            result->set_status(RedisStatus::TIMEOUT);
            result->set_err_desc("Err: Redis timeout");
            m_status = RedisStatus::INVALID;
            close();
            break;
        }
        offset += len;
        data[offset] = '\0';
        if (offset == buffer_size) {
            result->set_status(RedisStatus::READ_OVERFLOW);
            result->set_err_desc("Err: Redis read overflow");
            m_status = RedisStatus::INVALID;
            close();
            break;
        }
        result->parse(data, offset);
        if (result->parse_finished()) {
            m_last_rtime = Second();
            break;
        }
        if (result->is_parse_error()) {
            result->set_err_desc("Err: Redis parse data fail " + std::string(data, offset));
            m_status = RedisStatus::INVALID;
            close();
            break;
        }
    } while (true);
}

}  // namespace redis

}  // namespace tiger
