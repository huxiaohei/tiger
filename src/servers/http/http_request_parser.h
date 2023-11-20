/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_REQUEST_PARSER_H__
#define __TIGER_HTTP_HTTP_REQUEST_PARSER_H__

#include "http_parser_common.h"

namespace tiger
{

namespace http
{

typedef struct
{
    int cs;
    size_t body_start;
    int content_len;
    size_t nread;
    size_t mark;
    size_t field_start;
    size_t field_len;
    size_t query_start;
    int xml_sent;
    int json_sent;

    void *data;

    int uri_relaxed;
    field_cb http_field;
    element_cb request_method;
    element_cb request_uri;
    element_cb fragment;
    element_cb request_path;
    element_cb query_string;
    element_cb http_version;
    element_cb header_done;

} http_request_parser;

int http_request_parser_init(http_request_parser *parser);
int http_request_parser_finish(http_request_parser *parser);
size_t http_request_parser_execute(http_request_parser *parser, const char *data, size_t len, size_t off);
int http_request_parser_has_error(http_request_parser *parser);
int http_request_parser_is_finished(http_request_parser *parser);

#define http_request_parser_nread(parser) (parser)->nread

} // namespace http

} // namespace tiger

#endif