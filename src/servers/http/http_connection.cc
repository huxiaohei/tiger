/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_connection.h"

#include "../../streams/zlib_stream.h"

namespace tiger {

namespace http {

HTTPConnection::HTTPConnection(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {
    m_create_time = Millisecond();
    TIGER_LOG_I(SYSTEM_LOG) << "[connection construct"
                            << " create:" << m_create_time
                            << " socket:" << get_socket() << "]";
}

HTTPConnection::~HTTPConnection() {
    TIGER_LOG_I(SYSTEM_LOG) << "[connection destroy"
                            << " create:" << m_create_time
                            << " socket:" << get_socket() << "]";
}

HTTPResponse::ptr HTTPConnection::recv_response() {
    auto parser = std::make_shared<HTTPResponseParser>();
    uint64_t buffer_size = HTTPResponseParser::GetHTTPResponseBufferSize();
    std::shared_ptr<char> buffer(new char[buffer_size + 1], [](char *ptr) {
        delete[] ptr;
    });
    char *data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buffer_size - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);
        if (parser->has_error()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (int)buffer_size) {
            close();
            return nullptr;
        }
        if (parser->is_finished()) {
            break;
        }
    } while (true);

    const http_response_parser &rsp_parser = parser->get_parser();
    std::string body;
    if (rsp_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if (!begin || len == 0) {
                    int rt = read(data + len, buffer_size - len);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->has_error()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == (int)buffer_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->is_finished());
            if (rsp_parser.content_len + 2 <= len) {
                body.append(data, rsp_parser.content_len);
                memmove(data, data + rsp_parser.content_len + 2, len - rsp_parser.content_len - 2);
                len -= rsp_parser.content_len + 2;
            } else {
                body.append(data, len);
                int left = rsp_parser.content_len - len + 2;
                while (left > 0) {
                    int rt = read(data, left > (int)buffer_size ? (int)buffer_size : left);
                    if (rt < 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while (!rsp_parser.chunks_done);
    } else {
        int64_t length = parser->get_content_length();
        if (length > 0) {
            body.resize(length);
            int len = 0;
            if (length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            } else {
                memcpy(&body[0], data, length);
                len = offset;
            }
            length -= offset;
            if (length > 0) {
                if (read_fixed_size(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    if (!body.empty()) {
        auto encoding = parser->get_data()->get_header("content-encoding", "");
        if (strcasecmp(encoding.c_str(), "gzip") == 0) {
            auto zs = ZlibStream::CreateGzip(false);
            zs->write(body.c_str(), body.size());
            zs->flush();
            zs->get_result().swap(body);
        } else if (strcasecmp(encoding.c_str(), "deflate") == 0) {
            auto zs = ZlibStream::CreateDeflate(false);
            zs->write(body.c_str(), body.size());
            zs->flush();
            zs->get_result().swap(body);
        }
        parser->get_data()->set_body(body);
    }
    return parser->get_data();
}

int HTTPConnection::send_request(HTTPRequest::ptr req) {
    std::stringstream ss;
    ss << *req;
    std::string data = ss.str();
    return write_fixed_size(data.c_str(), data.size());
}

HTTPResult::ptr HTTPConnection::Get(const std::string &url,
                                    uint64_t timeout,
                                    const std::map<std::string, std::string> &headers,
                                    const std::string &body) {
    URI::ptr uri = URI::Create(url);
    if (!uri) {
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::INVALID_URL,
                                            "[invalid url:" + url + "]");
    }
    return Get(uri, timeout, headers, body);
}

HTTPResult::ptr HTTPConnection::Get(URI::ptr uri,
                                    uint64_t timeout,
                                    const std::map<std::string, std::string> &headers,
                                    const std::string &body) {
    return Request(HTTPMethod::GET, uri, timeout, headers, body);
}

HTTPResult::ptr HTTPConnection::Post(const std::string &url,
                                     uint64_t timeout,
                                     const std::map<std::string, std::string> &headers,
                                     const std::string &body) {
    URI::ptr uri = URI::Create(url);
    if (!uri) {
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::INVALID_URL,
                                            "[invalid url:" + url + "]");
    }
    return Request(HTTPMethod::POST, uri, timeout, headers, body);
}

HTTPResult::ptr HTTPConnection::Post(URI::ptr uri,
                                     uint64_t timeout,
                                     const std::map<std::string, std::string> &headers,
                                     const std::string &body) {
    return Request(HTTPMethod::POST, uri, timeout, headers, body);
}

HTTPResult::ptr HTTPConnection::Request(HTTPMethod method,
                                        const std::string &url,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    URI::ptr uri = URI::Create(url);
    if (!uri) {
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::INVALID_URL,
                                            "[invalid url: " + url + "]");
    }
    return Request(method, uri, timeout, headers, body);
}

HTTPResult::ptr HTTPConnection::Request(HTTPMethod method,
                                        URI::ptr uri,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    auto req = std::make_shared<HTTPRequest>();
    req->set_method(method);
    req->set_path(uri->get_path());
    req->set_query(uri->get_query());
    req->set_fragment(uri->get_fragment());
    bool has_host = false;
    for (auto &it : headers) {
        if (strcasecmp(it.first.c_str(), "connection") == 0) {
            if (strcasecmp(it.second.c_str(), "keep-alive") == 0) {
                req->set_close(false);
            }
            continue;
        }
        if (!has_host && strcasecmp(it.first.c_str(), "host") == 0) {
            has_host = !it.second.empty();
        }
        req->set_header(it.first, it.second);
    }
    if (!has_host) {
        req->set_header("Host", uri->get_host());
    }
    req->set_body(body);
    return Request(req, uri, timeout);
}

HTTPResult::ptr HTTPConnection::Request(HTTPRequest::ptr req,
                                        URI::ptr uri,
                                        uint64_t timeout) {
    bool is_ssl = uri->get_scheme() == "https";
    Address::ptr addr = uri->create_address();
    if (!addr) {
        std::stringstream ss;
        ss << "[Invalid host"
           << " host:" << uri->get_host()
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::INVALID_HOST,
                                            ss.str());
    }
    Socket::ptr socket = is_ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    if (!socket) {
        std::stringstream ss;
        ss << "[socket create fail"
           << " address:" << addr
           << " errno:" << strerror(errno)
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::CREATE_SOCKET_ERROR,
                                            ss.str());
    }
    if (!socket->connect(addr)) {
        std::stringstream ss;
        ss << "[socket connect fail"
           << " address:" << addr
           << " errno:" << strerror(errno)
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::CONNECT_FAIL,
                                            ss.str());
    }
    socket->set_recv_timeout(timeout);
    HTTPConnection::ptr conn = std::make_shared<HTTPConnection>(socket);
    int rt = conn->send_request(req);
    if (rt == 0) {
        std::stringstream ss;
        ss << "[socket closed fail"
           << " address:" << addr
           << " errno:" << strerror(errno)
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::SEND_CLOSE_BY_PEER,
                                            ss.str());
    }
    if (rt < 0) {
        std::stringstream ss;
        ss << "[socket send fail"
           << " address:" << addr
           << " errno:" << strerror(errno)
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::SEND_SOCKET_ERROR,
                                            ss.str());
    }
    HTTPResponse::ptr rsp = conn->recv_response();
    if (!rsp) {
        std::stringstream ss;
        ss << "[socket recv fail"
           << " address:" << addr
           << " errno:" << strerror(errno)
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::TIMEOUT,
                                            ss.str());
    }
    return std::make_shared<HTTPResult>(rsp,
                                        HTTPResult::ErrorCode::OK,
                                        "ok");
}

HTTPConnectionPool::ptr HTTPConnectionPool::Create(const std::string &uri_str,
                                                   const std::string &vhost,
                                                   uint32_t max_size,
                                                   uint32_t max_alive_time,
                                                   uint32_t max_request) {
    URI::ptr uri = URI::Create(uri_str);
    if (!uri) {
        TIGER_LOG_E(SYSTEM_LOG) << "[invalid uri " << uri_str << "]";
        return nullptr;
    }
    return std::make_shared<HTTPConnectionPool>(uri->get_host(),
                                                vhost,
                                                uri->get_scheme() == "https",
                                                uri->get_port(),
                                                max_size,
                                                max_alive_time,
                                                max_request);
}

HTTPConnectionPool::HTTPConnectionPool(const std::string &host,
                                       const std::string &vhost,
                                       bool is_https,
                                       uint32_t port,
                                       uint32_t max_size,
                                       uint32_t max_alive_time,
                                       uint32_t max_request)
    : m_host(host),
      m_vhost(vhost),
      m_is_https(is_https),
      m_port(port),
      m_max_size(max_size),
      m_max_alive_time(max_alive_time),
      m_max_request(max_request) {
}

HTTPConnection::ptr HTTPConnectionPool::get_connection() {
    uint64_t now = Millisecond();
    std::vector<HTTPConnection *> invalid_conns;
    HTTPConnection *ptr = nullptr;
    MutexLock::Lock lock(m_mutex);
    while (!m_conns.empty()) {
        HTTPConnection *conn = *m_conns.begin();
        m_conns.pop_front();
        if (!conn->is_connected()) {
            invalid_conns.push_back(conn);
            continue;
        }
        if ((conn->m_create_time + m_max_alive_time) < now) {
            invalid_conns.push_back(conn);
            continue;
        }
        ptr = conn;
        break;
    }
    lock.unlock();
    for (auto i : invalid_conns) {
        delete i;
    }
    m_total -= invalid_conns.size();

    if (!ptr) {
        auto addr = IPAddress::LookupAny(m_host);
        if (!addr) {
            TIGER_LOG_E(SYSTEM_LOG) << "[get ip address fail host:" << m_host << "]";
            return nullptr;
        }
        addr->set_port(m_port);
        Socket::ptr socket = m_is_https ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
        if (!socket) {
            TIGER_LOG_W(SYSTEM_LOG) << "[socket create fail addr:" << addr << "]";
            return nullptr;
        }
        if (!socket->connect(addr)) {
            TIGER_LOG_W(SYSTEM_LOG) << "[socket connect fail addr:" << addr << "]";
            return nullptr;
        }
        ptr = new HTTPConnection(socket);
        ++m_total;
    }
    return HTTPConnection::ptr(ptr,
                               std::bind(&HTTPConnectionPool::ReleasePtr,
                                         std::placeholders::_1,
                                         this));
}

HTTPResult::ptr HTTPConnectionPool::get(const std::string &path,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    return request(HTTPMethod::GET, path, timeout, headers, body);
}

HTTPResult::ptr HTTPConnectionPool::get(URI::ptr uri,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    std::stringstream ss;
    ss << uri->get_path()
       << (uri->get_query().empty() ? "" : "?") << uri->get_query()
       << (uri->get_fragment().empty() ? "" : "#") << uri->get_fragment();
    return get(ss.str(), timeout, headers, body);
}

HTTPResult::ptr HTTPConnectionPool::post(const std::string &path,
                                         uint64_t timeout,
                                         const std::map<std::string, std::string> &headers,
                                         const std::string &body) {
    return request(HTTPMethod::POST, path, timeout, headers, body);
}

HTTPResult::ptr HTTPConnectionPool::post(URI::ptr uri,
                                         uint64_t timeout,
                                         const std::map<std::string, std::string> &headers,
                                         const std::string &body) {
    std::stringstream ss;
    ss << uri->get_path()
       << (uri->get_query().empty() ? "" : "?") << uri->get_query()
       << (uri->get_fragment().empty() ? "" : "#") << uri->get_fragment();
    return post(ss.str(), timeout, headers, body);
}

HTTPResult::ptr HTTPConnectionPool::request(HTTPMethod method,
                                            const std::string &path,
                                            uint64_t timeout,
                                            const std::map<std::string, std::string> &headers,
                                            const std::string &body) {
    auto req = std::make_shared<HTTPRequest>();
    req->set_path(path);
    req->set_method(method);
    req->set_close(false);
    bool hasHost = false;
    for (auto &it : headers) {
        if (strcasecmp(it.first.c_str(), "connection") == 0) {
            if (strcasecmp(it.second.c_str(), "keep-alive") == 0) {
                req->set_close(false);
            }
            continue;
        }
        if (!hasHost && strcasecmp(it.first.c_str(), "host") == 0) {
            hasHost = !it.second.empty();
        }
        req->set_header(it.first, it.second);
    }
    if (!hasHost) {
        if (m_vhost.empty()) {
            req->set_header("Host", m_host);
        } else {
            req->set_header("Host", m_vhost);
        }
    }
    req->set_body(body);
    return request(req, timeout);
}

HTTPResult::ptr HTTPConnectionPool::request(HTTPMethod method,
                                            URI::ptr uri,
                                            uint64_t timeout,
                                            const std::map<std::string, std::string> &headers,
                                            const std::string &body) {
    std::stringstream ss;
    ss << uri->get_path()
       << (uri->get_query().empty() ? "" : "?") << uri->get_query()
       << (uri->get_fragment().empty() ? "" : "#") << uri->get_fragment();
    return request(method, ss.str(), timeout, headers, body);
}

HTTPResult::ptr HTTPConnectionPool::request(HTTPRequest::ptr req, uint64_t timeout) {
    auto conn = get_connection();
    if (!conn) {
        std::stringstream ss;
        ss << "[get connection from pool fail"
           << " req:" << req->to_string()
           << " timeout: " << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::POOL_GET_CONNECTION_FAIL,
                                            ss.str());
    }
    auto socket = conn->get_socket();
    if (!socket) {
        std::stringstream ss;
        ss << "[connection invalid"
           << " host:" << m_host
           << " port:" << m_port
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::POOL_INVALID_CONNECTION,
                                            ss.str());
    }
    socket->set_recv_timeout(timeout);
    int rt = conn->send_request(req);
    if (rt == 0) {
        std::stringstream ss;
        ss << "[socket closed"
           << " remote:" << socket->get_remote_addr()
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::SEND_CLOSE_BY_PEER,
                                            ss.str());
    }
    if (rt < 0) {
        std::stringstream ss;
        ss << "[socket send fail"
           << " remote:" << socket->get_remote_addr()
           << " timeout: " << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::SEND_SOCKET_ERROR,
                                            ss.str());
    }
    auto rsp = conn->recv_response();
    if (!rsp) {
        std::stringstream ss;
        ss << "[socket send fail"
           << " remote:" << socket->get_remote_addr()
           << " timeout:" << timeout << "]";
        return std::make_shared<HTTPResult>(nullptr,
                                            HTTPResult::ErrorCode::TIMEOUT,
                                            ss.str());
    }
    return std::make_shared<HTTPResult>(rsp,
                                        HTTPResult::ErrorCode::OK,
                                        "ok");
}

void HTTPConnectionPool::ReleasePtr(HTTPConnection *ptr, HTTPConnectionPool *pool) {
    ++ptr->m_request;
    if (!ptr->is_connected() ||
        (ptr->m_create_time + pool->m_max_alive_time) <= (uint64_t)Millisecond() ||
        ptr->m_request >= pool->m_max_request) {
        TIGER_LOG_D(SYSTEM_LOG) << "[connected:" << ptr->is_connected()
                                << " creatTime:" << ptr->m_create_time
                                << " aliveTime:" << pool->m_max_alive_time
                                << " request:" << ptr->m_request
                                << " maxRequest" << pool->m_max_request
                                << " total:" << pool->m_total << "]";
        delete ptr;
        --pool->m_total;
        return;
    }
    MutexLock::Lock lock(pool->m_mutex);
    pool->m_conns.push_back(ptr);
}

}  // namespace http

}  // namespace tiger