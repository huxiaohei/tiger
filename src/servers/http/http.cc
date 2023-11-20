/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http.h"

#include "../../util.h"

namespace tiger
{

namespace http
{

HTTPMethod StringToHTTPMethod(const std::string &m)
{
#define XX(num, name, string)                                                                                          \
    if (strcmp(#string, m.c_str()) == 0)                                                                               \
    {                                                                                                                  \
        return HTTPMethod::name;                                                                                       \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HTTPMethod::INVALID_METHOD;
}

HTTPMethod CharsToHTTPMethod(const char *m)
{
#define XX(num, name, string)                                                                                          \
    if (strncmp(#string, m, strlen(#string)) == 0)                                                                     \
    {                                                                                                                  \
        return HTTPMethod::name;                                                                                       \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HTTPMethod::INVALID_METHOD;
}

const char *HTTPMethodToString(const HTTPMethod &m)
{
    static const char *s_method_string[] = {
#define XX(num, name, string) #string,
        HTTP_METHOD_MAP(XX)
#undef XX
    };
    uint32_t idx = (uint32_t)m;
    if (idx > (sizeof(s_method_string) / sizeof(s_method_string[0])))
    {
        return "UNKNOWN";
    }
    return s_method_string[idx];
}

const char *HTTPStatusToString(const HTTPStatus &s)
{
    switch (s)
    {
#define XX(code, name, msg)                                                                                            \
    case HTTPStatus::name:                                                                                             \
        return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
    default:
        return "UNKNOWN";
    }
}

bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const
{
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HTTPResponse::HTTPResponse(uint8_t version, bool close)
    : m_status(HTTPStatus::OK), m_version(version), m_close(close), m_websocket(false)
{
}

void HTTPResponse::del_header(const std::string &key)
{
    if (m_headers.find(key) != m_headers.end())
        m_headers.erase(key);
}

void HTTPResponse::set_header(const std::string &key, const std::string &value)
{
    m_headers[key] = value;
}

void HTTPResponse::set_redirect(const std::string &uri)
{
    m_status = HTTPStatus::FOUND;
    set_header("Location", uri);
}

std::string HTTPResponse::get_header(const std::string &key, const std::string &def)
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

void HTTPResponse::set_cookies(const std::string &key, const std::string &val, time_t expired, const std::string &path,
                               const std::string &domain, bool secure)
{
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired > 0)
    {
        ss << ";expires=" << Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
    }
    if (!domain.empty())
    {
        ss << ";dlomain=" << domain;
    }
    if (!path.empty())
    {
        ss << ";path=" << path;
    }
    if (secure)
    {
        ss << ";secure";
    }
    m_cookies.push_back(ss.str());
}

std::string HTTPResponse::to_string() const
{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream &HTTPResponse::dump(std::ostream &os) const
{
    os << "HTTP/" << ((uint32_t)(m_version) >> 4) << "." << ((uint32_t)(m_version & 0x0F)) << " " << (uint32_t)m_status
       << " " << (m_reason.empty() ? HTTPStatusToString(m_status) : m_reason) << "\r\n";

    for (auto &it : m_headers)
    {
        if (!m_websocket && strcasecmp(it.first.c_str(), "connection") == 0)
        {
            continue;
        }
        os << it.first << ": " << it.second << "\r\n";
    }
    for (auto &it : m_cookies)
    {
        os << "Set-Cookie: " << it << "\r\n";
    }
    if (!m_websocket)
    {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    if (!m_body.empty())
    {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    }
    else
    {
        os << "\r\n";
    }
    return os;
}

HTTPRequest::HTTPRequest(uint8_t version, bool close)
    : m_method(HTTPMethod::GET), m_version(version), m_close(close), m_websocket(false), m_parser_param_flag(0),
      m_path("/")
{
}

void HTTPRequest::init()
{
    std::string conn = get_header("connection");
    if (!conn.empty())
    {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0)
        {
            m_close = false;
        }
        else
        {
            m_close = true;
        }
    }
}

void HTTPRequest::init_param()
{
    init_query_param();
    init_body_param();
    init_cookies();
}

#define PARSE_PARAM(str, m, flag, trim)                                                                                \
    size_t pos = 0;                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        size_t last = pos;                                                                                             \
        pos = str.find('=', pos);                                                                                      \
        if (pos == std::string::npos)                                                                                  \
        {                                                                                                              \
            break;                                                                                                     \
        }                                                                                                              \
        size_t key = pos;                                                                                              \
        pos = str.find(flag, pos);                                                                                     \
        m.insert(std::make_pair(trim(str.substr(last, key - last)),                                                    \
                                StringUtils::UrlEncode(str.substr(key + 1, pos - key - 1))));                          \
        if (pos == std::string::npos)                                                                                  \
        {                                                                                                              \
            break;                                                                                                     \
        }                                                                                                              \
        ++pos;                                                                                                         \
    } while (true);

void HTTPRequest::init_query_param()
{
    if (m_parser_param_flag & 0x1)
    {
        return;
    }
    PARSE_PARAM(m_query, m_params, '&', );
    m_parser_param_flag |= 0x1;
}

void HTTPRequest::init_body_param()
{
    if (m_parser_param_flag & 0x2)
    {
        return;
    }
    std::string content_type = get_header("content-type");
    if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr)
    {
        m_parser_param_flag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&', );
    m_parser_param_flag |= 0x2;
}

void HTTPRequest::init_cookies()
{
    if (m_parser_param_flag & 0x4)
    {
        return;
    }
    std::string cookie = get_header("cookie");
    if (cookie.empty())
    {
        m_parser_param_flag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ";", StringUtils::Trim);
    m_parser_param_flag |= 0x4;
}

std::string HTTPRequest::get_header(const std::string &key, const std::string &def) const
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

bool HTTPRequest::has_header(const std::string &key) const
{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? false : true;
}

void HTTPRequest::del_header(const std::string &key)
{
    m_headers.erase(key);
}

void HTTPRequest::set_header(const std::string &key, const std::string &val)
{
    m_headers[key] = val;
}

const std::map<std::string, std::string, CaseInsensitiveLess> &HTTPRequest::get_params()
{
    init_query_param();
    init_body_param();
    return m_params;
}

std::string HTTPRequest::get_param(const std::string &key, const std::string &def)
{
    init_query_param();
    init_body_param();
    auto it = m_params.find(key);
    return it == m_params.end() ? def : it->second;
}

bool HTTPRequest::has_param(const std::string &key)
{
    init_query_param();
    init_body_param();
    auto it = m_params.find(key);
    return it == m_params.end() ? false : true;
}

void HTTPRequest::del_param(const std::string &key)
{
    m_params.erase(key);
}

void HTTPRequest::set_param(const std::string &key, const std::string &val)
{
    m_params[key] = val;
}

const std::map<std::string, std::string, CaseInsensitiveLess> &HTTPRequest::get_cookies()
{
    init_cookies();
    return m_cookies;
}

std::string HTTPRequest::get_cookie(const std::string &key, const std::string &def)
{
    init_cookies();
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? def : it->second;
}

bool HTTPRequest::has_cookie(const std::string &key)
{
    init_cookies();
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? false : true;
}

void HTTPRequest::del_cookie(const std::string &key)
{
    if (m_cookies.find(key) == m_cookies.end())
        m_cookies.erase(key);
}

void HTTPRequest::set_cookie(const std::string &key, const std::string &val)
{
    m_cookies[key] = val;
}

std::ostream &HTTPRequest::dump(std::ostream &os) const
{
    os << HTTPMethodToString(m_method) << " " << m_path << (m_query.empty() ? "" : "?") << m_query
       << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/" << ((uint32_t)(m_version >> 4)) << "."
       << ((uint32_t)(m_version & 0x0F)) << "\r\n";
    if (!m_websocket)
    {
        os << "Connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    for (auto &it : m_headers)
    {
        if (!m_websocket && strcasecmp(it.first.c_str(), "connection") == 0)
        {
            continue;
        }
        os << it.first << ": " << it.second << "\r\n";
    }
    if (m_cookies.size() > 0)
    {
        os << "Set-Cookie: \n";
        for (auto &it : m_cookies)
        {
            os << "\t" << it.first << ":" << it.second << "\n";
        }
    }
    if (!m_body.empty())
    {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    }
    else
    {
        os << "\r\n";
    }
    return os;
}

std::string HTTPRequest::to_string() const
{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const HTTPRequest &request)
{
    return request.dump(os);
}

std::ostream &operator<<(std::ostream &os, const HTTPResponse &response)
{
    return response.dump(os);
}

std::ostream &operator<<(std::ostream &os, HTTPRequest::ptr request)
{
    return request->dump(os);
}

std::ostream &operator<<(std::ostream &os, HTTPResponse::ptr response)
{
    return response->dump(os);
}

} // namespace http

} // namespace tiger