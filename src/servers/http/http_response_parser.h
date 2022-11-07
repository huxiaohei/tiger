/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_RESPONSE_PARSER_H__
#define __TIGER_HTTP_HTTP_RESPONSE_PARSER_H__

#include "http_parser_common.h"

namespace tiger {

namespace http {

typedef struct {
    int cs;
    size_t body_start;
    int content_len;
    int status;
    int chunked;
    int chunks_done;
    int close;
    size_t nread;
    size_t mark;
    size_t field_start;
    size_t field_len;

    void *data;

    field_cb http_field;
    element_cb reason_phrase;
    element_cb status_code;
    element_cb chunk_size;
    element_cb http_version;
    element_cb header_done;
    element_cb last_chunk;

} http_response_parser;

int http_response_parser_init(http_response_parser *parser);
int http_response_parser_finish(http_response_parser *parser);
int http_response_parser_execute(http_response_parser *parser, const char *data, size_t len, size_t off);
int http_response_parser_has_error(http_response_parser *parser);
int http_response_parser_is_finished(http_response_parser *parser);

#define http_response_parser_nread(parser) (parser)->nread

}  // namespace http

}  // namespace tiger

#endif