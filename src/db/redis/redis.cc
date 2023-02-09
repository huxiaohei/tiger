/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "redis.h"

#include <cmath>

#include "../../macro.h"

namespace tiger {
namespace redis {

void RedisResultStr::parse(const char *s, int len) {
    if (len < 2) return;
    switch (s[0]) {
        case '-': {
            if (std::string(s, len - 2, 2) == CRLF) set_parse_finished(true);
            if (parse_finished()) {
                m_data = std::string(s, 1, len - 3);
            }
            set_status(RedisStatus::REPLY_ERROR);
            break;
        }
        case '+': {
            if (std::string(s, len - 2, 2) == CRLF) set_parse_finished(true);
            if (parse_finished()) {
                m_data = std::string(s, 1, len - 3);
            }
            set_status(RedisStatus::OK);
            break;
        }
        case '$': {
            int digit = std::atoi(s + 1);
            if (digit == -1) {
                set_parse_finished(true);
                set_status(RedisStatus::NIL_ERROR);
            } else {
                if (len - digit - 2 - 2 - 1 == 1 + (int)floor(log10(digit))) {
                    set_parse_finished(true);
                    set_status(RedisStatus::OK);
                    m_data = std::string(s, len - digit - 2, digit);
                } else {
                    set_parse_finished(false);
                }
            }
            break;
        }
        default:
            set_parse_finished(true);
            set_status(RedisStatus::PARSE_ERROR);
            break;
    }
    TIGER_LOG_D(TEST_LOG) << m_data;
}

}  // namespace redis
}  // namespace tiger