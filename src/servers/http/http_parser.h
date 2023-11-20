/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_PARSER_H__
#define __TIGER_HTTP_HTTP_PARSER_H__

#include "http.h"
#include "http_request_parser.h"
#include "http_response_parser.h"

namespace tiger
{

namespace http
{

enum HTTPParserError
{
    OK = 0,
    INVALID_METHOD_ERROR = -1001,
    INVALID_VERSION_ERROR,
    INVALID_FIELD_ERROR
};

class HTTPRequestParser
{
  private:
    http_request_parser m_parser;
    HTTPRequest::ptr m_data;
    HTTPParserError m_error;

  public:
    typedef std::shared_ptr<HTTPRequestParser> ptr;

    HTTPRequestParser();

  public:
    int is_finished();

    int has_error();
    void set_error(HTTPParserError v)
    {
        m_error = v;
    }

    HTTPRequest::ptr get_data() const
    {
        return m_data;
    }

    const http_request_parser &get_parser() const
    {
        return m_parser;
    }

    uint64_t get_content_length();

    size_t execute(char *data, size_t len);

  public:
    static uint64_t GetHTTPRequestBufferSize();
    static uint64_t GetHTTPRequestMaxBodySize();
};

class HTTPResponseParser
{
  private:
    http_response_parser m_parser;
    HTTPResponse::ptr m_data;
    HTTPParserError m_error;

  public:
    typedef std::shared_ptr<HTTPResponseParser> ptr;

    HTTPResponseParser();

  public:
    int is_finished();

    int has_error();
    void set_error(HTTPParserError v)
    {
        m_error = v;
    }

    HTTPResponse::ptr get_data() const
    {
        return m_data;
    }

    const http_response_parser &get_parser() const
    {
        return m_parser;
    }

    uint64_t get_content_length();

    size_t execute(char *data, size_t len, bool chunck);

  public:
    static uint64_t GetHTTPResponseBufferSize();
    static uint64_t GetHTTPResponseMaxBodySize();
};

} // namespace http
} // namespace tiger

#endif