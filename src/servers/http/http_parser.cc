/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_parser.h"

#include "../../config.h"

namespace tiger
{

namespace http
{

static ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    Config::Lookup<uint64_t>("tiger.servers.http.requestBufferSize", 4 * 1024, "HTTP request buffer size");
static ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    Config::Lookup<uint64_t>("tiger.servers.http.requestMaxBodySize", 64 * 1024 * 1024, "HTTP request max body size");
static ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    Config::Lookup<uint64_t>("tiger.servers.http.responseBufferSize", 4 * 1024, "HTTP response buffer size");
static ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    Config::Lookup<uint64_t>("tiger.servers.http.responseMaxBodySize", 64 * 1024 * 1024, "HTTP response max body size");

void on_request_method(void *data, const char *at, size_t length)
{
    HTTPRequestParser *parser = static_cast<HTTPRequestParser *>(data);
    HTTPMethod m = CharsToHTTPMethod(at);
    if (m == HTTPMethod::INVALID_METHOD)
    {
        TIGER_LOG_W(SYSTEM_LOG) << "[invalid http method"
                                << " method:" << std::string(at, length) << "]";
        parser->set_error(HTTPParserError::INVALID_METHOD_ERROR);
        return;
    }
    parser->get_data()->set_method(m);
}

void on_request_uri(void *data, const char *at, size_t length)
{
}

void on_request_fragment(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPRequestParser *>(data);
    parser->get_data()->set_fragment(std::string(at, length));
}

void on_request_path(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPRequestParser *>(data);
    parser->get_data()->set_path(std::string(at, length));
}

void on_request_query_string(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPRequestParser *>(data);
    parser->get_data()->set_query(std::string(at, length));
}

void on_request_http_version(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPRequestParser *>(data);
    uint8_t v = 0;
    if (strncmp(at, "HTTP/1.1", length) == 0)
    {
        v = 0x11;
    }
    else if (strncmp(at, "HTTP/1.0", length) == 0)
    {
        v = 0x10;
    }
    else
    {
        TIGER_LOG_W(SYSTEM_LOG) << "[invalid http version"
                                << " version:" << std::string(at, length) << "]";
        parser->set_error(HTTPParserError::INVALID_VERSION_ERROR);
        return;
    }
    parser->get_data()->set_version(v);
}

void on_request_header_done(void *data, const char *at, size_t length)
{
}

void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
    auto parser = static_cast<HTTPRequestParser *>(data);
    if (flen == 0)
    {
        TIGER_LOG_W(SYSTEM_LOG) << "[invalid http field]";
        return;
    }
    parser->get_data()->set_header(std::string(field, flen), std::string(value, vlen));
}

HTTPRequestParser::HTTPRequestParser() : m_error(HTTPParserError::OK)
{
    m_data.reset(new HTTPRequest);
    http_request_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query_string;
    m_parser.http_version = on_request_http_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    m_parser.data = this;
}

size_t HTTPRequestParser::execute(char *data, size_t len)
{
    size_t offset = http_request_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

int HTTPRequestParser::is_finished()
{
    return http_request_parser_finish(&m_parser);
}

int HTTPRequestParser::has_error()
{
    return m_error || http_request_parser_has_error(&m_parser);
}

uint64_t HTTPRequestParser::get_content_length()
{
    return m_data->get_header_as<uint64_t>("content-length", 0);
}

uint64_t HTTPRequestParser::GetHTTPRequestBufferSize()
{
    return g_http_response_buffer_size->val();
}

uint64_t HTTPRequestParser::GetHTTPRequestMaxBodySize()
{
    return g_http_request_max_body_size->val();
}

void on_response_reason_phrase(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPResponseParser *>(data);
    parser->get_data()->set_reason(std::string(at, length));
}

void on_response_status_code(void *data, const char *at, size_t length)
{
    auto parser = static_cast<HTTPResponseParser *>(data);
    parser->get_data()->set_status((HTTPStatus)atoi(at));
}

void on_response_chunk_size(void *data, const char *at, size_t length)
{
}

void on_response_http_version(void *data, const char *at, size_t length)
{
    HTTPResponseParser *parser = static_cast<HTTPResponseParser *>(data);
    uint8_t v = 0x11;
    if (strncmp(at, "HTTP/1.1", length) == 0)
    {
        v = 0x11;
    }
    else if (strncmp(at, "HTTP/1.0", length) == 0)
    {
        v = 0x10;
    }
    else
    {
        TIGER_LOG_W(SYSTEM_LOG) << "[invalid http version"
                                << " version:" << std::string(at, length) << "]";
        parser->set_error(HTTPParserError::INVALID_VERSION_ERROR);
        return;
    }
    parser->get_data()->set_version(v);
}

void on_response_header_done(void *data, const char *at, size_t length)
{
}

void on_response_last_chunk(void *data, const char *at, size_t length)
{
}

void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
{
    HTTPResponseParser *parse = static_cast<HTTPResponseParser *>(data);
    if (flen == 0)
    {
        TIGER_LOG_W(SYSTEM_LOG) << "[invalid http field]";
        return;
    }
    parse->get_data()->set_header(std::string(field, flen), std::string(value, vlen));
}

HTTPResponseParser::HTTPResponseParser() : m_error(HTTPParserError::OK)
{
    m_data = std::make_shared<HTTPResponse>();
    http_response_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason_phrase;
    m_parser.status_code = on_response_status_code;
    m_parser.chunk_size = on_response_chunk_size;
    m_parser.http_version = on_response_http_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data = this;
}

size_t HTTPResponseParser::execute(char *data, size_t len, bool chunck)
{
    if (chunck)
    {
        http_response_parser_init(&m_parser);
    }
    size_t offset = http_response_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

int HTTPResponseParser::is_finished()
{
    return http_response_parser_is_finished(&m_parser);
}

int HTTPResponseParser::has_error()
{
    return m_error || http_response_parser_has_error(&m_parser);
}

uint64_t HTTPResponseParser::get_content_length()
{
    return m_data->get_header_as<uint64_t>("content-length", 0);
}

uint64_t HTTPResponseParser::GetHTTPResponseBufferSize()
{
    return g_http_response_buffer_size->val();
}

uint64_t HTTPResponseParser::GetHTTPResponseMaxBodySize()
{
    return g_http_response_max_body_size->val();
}

} // namespace http
} // namespace tiger