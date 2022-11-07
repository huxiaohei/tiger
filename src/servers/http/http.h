/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_H__
#define __TIGER_HTTP_HTTP_H__

#include <sys/types.h>

#include <boost/lexical_cast.hpp>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "../../const.h"
#include "../../macro.h"

namespace tiger {

namespace http {

/* Request Methods */
#define HTTP_METHOD_MAP(XX)          \
    XX(0, DELETE, DELETE)            \
    XX(1, GET, GET)                  \
    XX(2, HEAD, HEAD)                \
    XX(3, POST, POST)                \
    XX(4, PUT, PUT)                  \
    /* pathological */               \
    XX(5, CONNECT, CONNECT)          \
    XX(6, OPTIONS, OPTIONS)          \
    XX(7, TRACE, TRACE)              \
    /* WebDAV */                     \
    XX(8, COPY, COPY)                \
    XX(9, LOCK, LOCK)                \
    XX(10, MKCOL, MKCOL)             \
    XX(11, MOVE, MOVE)               \
    XX(12, PROPFIND, PROPFIND)       \
    XX(13, PROPPATCH, PROPPATCH)     \
    XX(14, SEARCH, SEARCH)           \
    XX(15, UNLOCK, UNLOCK)           \
    XX(16, BIND, BIND)               \
    XX(17, REBIND, REBIND)           \
    XX(18, UNBIND, UNBIND)           \
    XX(19, ACL, ACL)                 \
    /* subversion */                 \
    XX(20, REPORT, REPORT)           \
    XX(21, MKACTIVITY, MKACTIVITY)   \
    XX(22, CHECKOUT, CHECKOUT)       \
    XX(23, MERGE, MERGE)             \
    /* upnp */                       \
    XX(24, MSEARCH, M - SEARCH)      \
    XX(25, NOTIFY, NOTIFY)           \
    XX(26, SUBSCRIBE, SUBSCRIBE)     \
    XX(27, UNSUBSCRIBE, UNSUBSCRIBE) \
    /* RFC-5789 */                   \
    XX(28, PATCH, PATCH)             \
    XX(29, PURGE, PURGE)             \
    /* CalDAV */                     \
    XX(30, MKCALENDAR, MKCALENDAR)   \
    /* RFC-2068, section 19.6.1.2 */ \
    XX(31, LINK, LINK)               \
    XX(32, UNLINK, UNLINK)           \
    /* icecast */                    \
    XX(33, SOURCE, SOURCE)

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                   \
    XX(100, CONTINUE, Continue)                                               \
    XX(101, SWITCHING_PROTOCOLS, Switching Protocols)                         \
    XX(102, PROCESSING, Processing)                                           \
    XX(200, OK, OK)                                                           \
    XX(201, CREATED, Created)                                                 \
    XX(202, ACCEPTED, Accepted)                                               \
    XX(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)   \
    XX(204, NO_CONTENT, No Content)                                           \
    XX(205, RESET_CONTENT, Reset Content)                                     \
    XX(206, PARTIAL_CONTENT, Partial Content)                                 \
    XX(207, MULTI_STATUS, Multi - Status)                                     \
    XX(208, ALREADY_REPORTED, Already Reported)                               \
    XX(226, IM_USED, IM Used)                                                 \
    XX(300, MULTIPLE_CHOICES, Multiple Choices)                               \
    XX(301, MOVED_PERMANENTLY, Moved Permanently)                             \
    XX(302, FOUND, Found)                                                     \
    XX(303, SEE_OTHER, See Other)                                             \
    XX(304, NOT_MODIFIED, Not Modified)                                       \
    XX(305, USE_PROXY, Use Proxy)                                             \
    XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                           \
    XX(308, PERMANENT_REDIRECT, Permanent Redirect)                           \
    XX(400, BAD_REQUEST, Bad Request)                                         \
    XX(401, UNAUTHORIZED, Unauthorized)                                       \
    XX(402, PAYMENT_REQUIRED, Payment Required)                               \
    XX(403, FORBIDDEN, Forbidden)                                             \
    XX(404, NOT_FOUND, Not Found)                                             \
    XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                           \
    XX(406, NOT_ACCEPTABLE, Not Acceptable)                                   \
    XX(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)     \
    XX(408, REQUEST_TIMEOUT, Request Timeout)                                 \
    XX(409, CONFLICT, Conflict)                                               \
    XX(410, GONE, Gone)                                                       \
    XX(411, LENGTH_REQUIRED, Length Required)                                 \
    XX(412, PRECONDITION_FAILED, Precondition Failed)                         \
    XX(413, PAYLOAD_TOO_LARGE, Payload Too Large)                             \
    XX(414, URI_TOO_LONG, URI Too Long)                                       \
    XX(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                   \
    XX(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                     \
    XX(417, EXPECTATION_FAILED, Expectation Failed)                           \
    XX(421, MISDIRECTED_REQUEST, Misdirected Request)                         \
    XX(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                       \
    XX(423, LOCKED, Locked)                                                   \
    XX(424, FAILED_DEPENDENCY, Failed Dependency)                             \
    XX(426, UPGRADE_REQUIRED, Upgrade Required)                               \
    XX(428, PRECONDITION_REQUIRED, Precondition Required)                     \
    XX(429, TOO_MANY_REQUESTS, Too Many Requests)                             \
    XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
    XX(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)     \
    XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                     \
    XX(501, NOT_IMPLEMENTED, Not Implemented)                                 \
    XX(502, BAD_GATEWAY, Bad Gateway)                                         \
    XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                         \
    XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                 \
    XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)           \
    XX(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                 \
    XX(507, INSUFFICIENT_STORAGE, Insufficient Storage)                       \
    XX(508, LOOP_DETECTED, Loop Detected)                                     \
    XX(510, NOT_EXTENDED, Not Extended)                                       \
    XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

enum class HTTPMethod {
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
        INVALID_METHOD
};

enum class HTTPStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

HTTPMethod StringToHTTPMethod(const std::string &m);
HTTPMethod CharsToHTTPMethod(const char *m);

const char *HTTPMethodToString(const HTTPMethod &m);
const char *HTTPStatusToString(const HTTPStatus &s);

struct CaseInsensitiveLess {
    bool operator()(const std::string &lhs, const std::string &rhs) const;
};

template <class T>
bool CheckGetAs(const std::map<std::string, std::string, CaseInsensitiveLess> &m, const std::string &key, T &val, const T def = T()) {
    auto it = m.find(key);
    if (it == m.end()) {
        val = def;
        return false;
    }
    try {
        val = boost::lexical_cast<T>(it->second);
        return false;
    } catch (const std::exception &e) {
        val = def;
        TIGER_LOG_W(SYSTEM_LOG) << "[CheckGetAs fail"
                                << " key:" << key
                                << " what:" << e.what() << "]";
    }
    return false;
}

template <class T>
T GetAs(const std::map<std::string, std::string, CaseInsensitiveLess> &m, const std::string &key, const T &def = T()) {
    auto it = m.find(key);
    if (it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<T>(it->second);
    } catch (const std::exception &e) {
        TIGER_LOG_W(SYSTEM_LOG) << "[GetAs fail"
                                << " key:" << key
                                << " what:" << e.what() << "]";
    }
    return def;
}

class HTTPResponse {
   private:
    HTTPStatus m_status;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;
    std::string m_body;
    std::string m_reason;
    std::map<std::string, std::string, CaseInsensitiveLess> m_headers;
    std::vector<std::string> m_cookies;

   public:
    typedef std::shared_ptr<HTTPResponse> ptr;

    HTTPResponse(uint8_t version = 0x11, bool close = true);

   public:
    HTTPStatus get_status() const { return m_status; }
    void set_status(HTTPStatus v) { m_status = v; }

    uint8_t get_version() const { return m_version; }
    void set_version(uint8_t v) { m_version = v; }

    bool is_close() const { return m_close; }
    void set_close(bool v) { m_close = v; }

    bool is_websocket() const { return m_websocket; }
    void set_websocket(bool v) { m_websocket = v; }

    const std::string &get_body() const { return m_body; }
    void set_body(const std::string &v) { m_body = v; }

    const std::string &get_reason() const { return m_reason; }
    void set_reason(const std::string &v) { m_reason = v; }

    const std::map<std::string, std::string, CaseInsensitiveLess> &get_headers() { return m_headers; }
    std::string get_header(const std::string &key, const std::string &v);
    void set_headers(const std::map<std::string, std::string, CaseInsensitiveLess> &v) { m_headers = v; }
    void set_header(const std::string &key, const std::string &value);
    void del_header(const std::string &key);

    template <class T>
    bool check_get_header_as(const std::string &key, T &val, const T &def = T()) {
        return CheckGetAs(m_headers, key, val, def);
    }

    template <class T>
    T get_header_as(const std::string &key, const T &def = T()) {
        return GetAs(m_headers, key, def);
    }

    void set_redirect(const std::string &uri);

    void set_cookies(const std::string &key, const std::string &val,
                     time_t expired = 0, const std::string &path = "",
                     const std::string &domain = "", bool secure = false);

    std::string to_string() const;
    std::ostream &dump(std::ostream &os) const;
};

class HTTPRequest {
   private:
    HTTPMethod m_method;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;
    uint8_t m_parser_param_flag;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_body;
    std::map<std::string, std::string, CaseInsensitiveLess> m_headers;
    std::map<std::string, std::string, CaseInsensitiveLess> m_params;
    std::map<std::string, std::string, CaseInsensitiveLess> m_cookies;

   public:
    typedef std::shared_ptr<HTTPRequest> ptr;

    HTTPRequest(uint8_t version = 0x11, bool close = true);

   public:
    HTTPMethod get_method() const { return m_method; }
    void set_method(HTTPMethod v) { m_method = v; }

    uint8_t get_version() const { return m_version; }
    void set_version(uint8_t v) { m_version = v; }

    bool is_close() const { return m_close; }
    void set_close(bool v) { m_close = v; }

    bool is_websocket() const { return m_websocket; }
    void set_websocket(bool v) { m_websocket = v; }

    uint8_t get_parser_param_flag() const { return m_parser_param_flag; }
    void set_parser_param_flag(uint8_t v) { m_parser_param_flag = v; }

    const std::string &get_path() const { return m_path; }
    void set_path(const std::string &v) { m_path = v; }

    const std::string &get_query() const { return m_query; }
    void set_query(const std::string &v) { m_query = v; }

    const std::string &get_fragment() const { return m_fragment; }
    void set_fragment(const std::string &v) { m_fragment = v; }

    const std::string &get_body() const { return m_body; }
    void set_body(const std::string &v) { m_body = v; }

    const std::map<std::string, std::string, CaseInsensitiveLess> &get_headers() const { return m_headers; }
    std::string get_header(const std::string &key, const std::string &def = "") const;
    void set_headers(std::map<std::string, std::string, CaseInsensitiveLess> &v) { m_headers = v; }
    void set_header(const std::string &key, const std::string &val);
    bool has_header(const std::string &key) const;
    void del_header(const std::string &key);

    template <class T>
    bool check_get_header_as(const std::string &key, T &v, const T &def = T()) {
        return CheckGetAs(m_headers, key, v, def);
    }
    template <class T>
    T get_header_as(const std::string &key, const T &def = T()) {
        return GetAs(m_headers, key, def);
    }

    std::string get_param(const std::string &key, const std::string &def = "");
    const std::map<std::string, std::string, CaseInsensitiveLess> &get_params();
    void set_params(std::map<std::string, std::string, CaseInsensitiveLess> &v) { m_params = v; }
    void set_param(const std::string &key, const std::string &val);
    bool has_param(const std::string &key);
    void del_param(const std::string &key);

    template <class T>
    bool check_get_param_as(const std::string &key, T &v, const T &def = T()) {
        return CheckGetAs(m_params, key, v, def);
    }
    template <class T>
    T check_get_param_as(const std::string &key, const T &def = T()) {
        return GetAs(m_params, key, def);
    }

    void set_cookies(std::map<std::string, std::string, CaseInsensitiveLess> &v) { m_cookies = v; }

    const std::map<std::string, std::string, CaseInsensitiveLess> &get_cookies();
    std::string get_cookie(const std::string &key, const std::string &def = "");
    bool has_cookie(const std::string &key);
    void del_cookie(const std::string &key);
    void set_cookie(const std::string &key, const std::string &val);
    template <class T>
    bool check_get_cookie_as(const std::string &key, T &v, const T &def = T()) {
        return CheckGetAs(m_cookies, key, v, def);
    }
    template <class T>
    T get_cookie_as(const std::string &key, const T &def = T()) {
        return GetAs(m_cookies, key, def);
    }

    std::ostream &dump(std::ostream &os) const;
    std::string to_string() const;

   public:
    void init();
    void init_param();
    void init_query_param();
    void init_body_param();
    void init_cookies();
};

std::ostream &operator<<(std::ostream &os, const HTTPRequest &request);
std::ostream &operator<<(std::ostream &os, const HTTPResponse &response);
std::ostream &operator<<(std::ostream &os, HTTPRequest::ptr request);
std::ostream &operator<<(std::ostream &os, HTTPResponse::ptr response);

}  // namespace http

}  // namespace tiger

#endif