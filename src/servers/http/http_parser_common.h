/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_PARSER_COMMON_H__
#define __TIGER_HTTP_HTTP_PARSER_COMMON_H__

#include <sys/types.h>

namespace tiger
{

namespace http
{

typedef void (*element_cb)(void *data, const char *at, size_t len);
typedef void (*field_cb)(void *data, const char *field, size_t f_len, const char *value, size_t v_len);

} // namespace http

} // namespace tiger

#endif