/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_CONNECTION_H__
#define __TIGER_HTTP_HTTP_CONNECTION_H__

#include <atomic>
#include <list>

#include "../../streams/socket_stream.h"
#include "../../thread.h"
#include "../../uri.h"
#include "http.h"
#include "http_parser.h"

namespace tiger
{

namespace http
{

struct HTTPResult
{
    typedef std::shared_ptr<HTTPResult> ptr;

    typedef enum
    {
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE_BY_PEER = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
        CREATE_SOCKET_ERROR = 7,
        POOL_GET_CONNECTION_FAIL = 8,
        POOL_INVALID_CONNECTION = 9,
        INVALID_PROTOCOLS = 10
    } ErrorCode;

    HTTPResponse::ptr rsp;
    ErrorCode err_code;
    std::string err_des;

    HTTPResult(HTTPResponse::ptr _rsp, ErrorCode _err_code = ErrorCode::OK, const std::string &_err_des = "")
        : rsp(_rsp), err_code(_err_code), err_des(_err_des)
    {
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[errCode:" << err_code << " errDes:" << err_des << " rsp:[\n" << rsp->to_string() << "\n]]";
        return ss.str();
    }
};

class HTTPConnectionPool;

class HTTPConnection : public SocketStream
{
    friend class HTTPConnectionPool;

  private:
    uint64_t m_create_time = 0;
    uint64_t m_request = 0;

  public:
    typedef std::shared_ptr<HTTPConnection> ptr;

    HTTPConnection(Socket::ptr sock, bool owner = true);
    ~HTTPConnection();

    HTTPResponse::ptr recv_response();
    int send_request(HTTPRequest::ptr req);

  public:
    static HTTPResult::ptr Get(const std::string &url, uint64_t timeout,
                               const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    static HTTPResult::ptr Get(URI::ptr uri, uint64_t timeout, const std::map<std::string, std::string> &headers = {},
                               const std::string &body = "");

    static HTTPResult::ptr Post(const std::string &url, uint64_t timeout,
                                const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    static HTTPResult::ptr Post(URI::ptr uri, uint64_t timeout, const std::map<std::string, std::string> &headers = {},
                                const std::string &body = "");

    static HTTPResult::ptr Request(HTTPMethod method, const std::string &url, uint64_t timeout,
                                   const std::map<std::string, std::string> &headers = {},
                                   const std::string &body = "");

    static HTTPResult::ptr Request(HTTPMethod method, URI::ptr uri, uint64_t timeout,
                                   const std::map<std::string, std::string> &headers = {},
                                   const std::string &body = "");

    static HTTPResult::ptr Request(HTTPRequest::ptr req, URI::ptr uri, uint64_t timeout);
};

class HTTPConnectionPool
{
  private:
    std::string m_host;
    std::string m_vhost;
    bool m_is_https;
    uint32_t m_port;
    uint32_t m_max_size;
    uint32_t m_max_alive_time;
    uint32_t m_max_request;

    MutexLock m_mutex;
    std::list<HTTPConnection *> m_conns;
    std::atomic<int32_t> m_total = {0};

  private:
    static void ReleasePtr(HTTPConnection *ptr, HTTPConnectionPool *pool);

  public:
    typedef std::shared_ptr<HTTPConnectionPool> ptr;

    static HTTPConnectionPool::ptr Create(const std::string &uri_str, const std::string &vhost, uint32_t max_size,
                                          uint32_t max_alive_time, uint32_t max_request);

    HTTPConnectionPool(const std::string &host, const std::string &v_host, bool is_https, uint32_t port,
                       uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);

  public:
    HTTPConnection::ptr get_connection();

  public:
    HTTPResult::ptr get(const std::string &path, uint64_t timeout,
                        const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    HTTPResult::ptr get(URI::ptr uri, uint64_t timeout, const std::map<std::string, std::string> &headers = {},
                        const std::string &body = "");

    HTTPResult::ptr post(const std::string &path, uint64_t timeout,
                         const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    HTTPResult::ptr post(URI::ptr uri, uint64_t timeout, const std::map<std::string, std::string> &headers = {},
                         const std::string &body = "");

    HTTPResult::ptr request(HTTPMethod method, const std::string &path, uint64_t timeout,
                            const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    HTTPResult::ptr request(HTTPMethod method, URI::ptr uri, uint64_t timeout,
                            const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

    HTTPResult::ptr request(HTTPRequest::ptr req, uint64_t timeout);
};

} // namespace http
} // namespace tiger

#endif