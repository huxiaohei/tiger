/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REIDS_H__
#define __TIGER_REDIS_REIDS_H__

#include <cmath>
#include <memory>
#include <string>

#include "../../macro.h"

namespace tiger {

namespace redis {

#define TIGER_REDIS_CRLF "\r\n"
#define TIGER_REDIS_CMD_AUTH "AUTH"
#define TIGER_REDIS_CMD_PING "PING"
#define TIGER_REDIS_CMD_KEY "KEY"
#define TIGER_REDIS_CMD_GET "GET"

enum RedisStatus {
    OK = 0,
    INVALID = -1,
    CONNECT_FAIL = -2,
    AUTH_FAIL = -3,
    REPLY_ERROR = -4,
    NIL_ERROR = -5,
    PARSE_ERROR = -5,
    READ_OVERFLOW = -7
};

template <typename T>
class RedisValTrans {
   public:
    T operator()(const char *s, int len) {
        return T();
    }
};

template <>
class RedisValTrans<std::string> {
   public:
    std::string operator()(const char *s, int len) {
        return std::string(s, len);
    }
};

template <>
class RedisValTrans<int> {
   public:
    int operator()(const char *s, int len) {
        return std::atoi(s);
    }
};

class RedisResult {
   private:
    RedisStatus m_status;
    bool m_parse_finished;

   protected:
    std::string m_err_desc;

   public:
    typedef std::shared_ptr<RedisResult> ptr;

    RedisResult() : m_status(RedisStatus::INVALID), m_parse_finished(false), m_err_desc("") {}
    virtual ~RedisResult(){};

   public:
    void set_status(RedisStatus s) { m_status = s; };
    RedisStatus get_status() const { return m_status; }
    void set_parse_finished(bool v) { m_parse_finished = true; }

    bool parse_finished() const { return m_parse_finished; }
    bool is_ok() { return m_status == RedisStatus::OK; }
    bool is_parse_error() { return m_status == RedisStatus::PARSE_ERROR; }

   public:
    virtual void parse(const char *s, int len) = 0;
};

class RedisResultStr : public RedisResult {
   private:
    std::string m_data;

   public:
    typedef std::shared_ptr<RedisResultStr> ptr;

   public:
    std::string &get_data() { return m_data; };

    friend std::ostream &operator<<(std::ostream &os, const RedisResultStr &rst) {
        return os << rst.m_data;
    }

    friend std::ostream &operator<<(std::ostream &os, const RedisResultStr::ptr rst) {
        return os << rst->m_data;
    }

   public:
    void parse(const char *s, int len) override {
        if (len < 2) return;
        switch (s[0]) {
            case '-': {
                if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) set_parse_finished(true);
                if (parse_finished()) {
                    m_err_desc = std::string(s, 1, len - 3);
                }
                set_status(RedisStatus::REPLY_ERROR);
                break;
            }
            case '+': {
                if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) set_parse_finished(true);
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
                    m_err_desc = "nil";
                    m_data.clear();
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
    }
};

template <typename T>
class RedisResultVector : public RedisResult {
   private:
    std::vector<T> m_data;
    int m_parse_len;

   public:
    typedef std::shared_ptr<RedisResultVector<T>> ptr;

    RedisResultVector()
        : RedisResult(), m_parse_len(0){};

   public:
    std::vector<T> &get_data() { return m_data; };

    friend std::ostream &operator<<(std::ostream &os, const RedisResultVector<T> &rst) {
        os << "[";
        for (const auto &it : rst.m_data) {
            os << it << ",";
        }
        os << "]";
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, const RedisResultVector<T>::ptr rst) {
        os << "[";
        for (const auto &it : rst->m_data) {
            os << it << ",";
        }
        os << "]";
        return os;
    }

   public:
    void parse(const char *s, int len) override {
        if (len < 2) return;
        switch (s[0]) {
            case '-': {
                if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) set_parse_finished(true);
                if (parse_finished()) {
                    m_data.clear();
                    m_err_desc = std::string(s, 1, len - 3);
                }
                set_status(RedisStatus::REPLY_ERROR);
                break;
            }
            case '*': {
                int rst_size = std::atoi(s + 1);
                if (m_parse_len == 0) {
                    m_parse_len += (1 + 1 + (int)floor(log10(rst_size)) + 2);
                }
                while (m_parse_len < len) {
                    int item_len = std::atoi(s + m_parse_len + 1);
                    m_parse_len += (1 + 1 + (int)floor(log10(item_len)) + 2);
                    if (m_parse_len + item_len + 2 > len) {
                        m_parse_len -= (1 + 1 + (int)floor(log10(item_len)) + 2);
                        break;
                    }
                    m_data.push_back(RedisValTrans<T>()(s + m_parse_len, item_len));
                    m_parse_len += (item_len + 2);
                }
                if (rst_size == (int)m_data.size() && m_parse_len == len) {
                    set_parse_finished(true);
                    set_status(RedisStatus::OK);
                }
                break;
            }
            case '$': {
                int digit = std::atoi(s + 1);
                if (digit == -1) {
                    set_parse_finished(true);
                    set_status(RedisStatus::NIL_ERROR);
                    m_data.clear();
                    m_err_desc = "nil";
                } else {
                    set_parse_finished(true);
                    set_status(RedisStatus::PARSE_ERROR);
                }
                break;
            }
            default:
                set_parse_finished(true);
                set_status(RedisStatus::PARSE_ERROR);
                break;
        }
    }
};

}  // namespace redis

}  // namespace tiger

#endif