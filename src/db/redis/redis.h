/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REIDS_H__
#define __TIGER_REDIS_REIDS_H__

#include <boost/lexical_cast.hpp>
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
    NIL_ERROR = 2,
    OK = 1,
    INVALID = -1,
    CONNECT_FAIL = -2,
    AUTH_FAIL = -3,
    REPLY_ERROR = -4,
    PARSE_ERROR = -5,
    READ_OVERFLOW = -6,
    TIMEOUT = -7
};

template <typename T>
class RedisValTrans {
   public:
    T operator()(const char *s, int len) {
        return boost::lexical_cast<T>(std::string(s, len));
    }
};

template <>
class RedisValTrans<bool> {
   public:
    bool operator()(const char *s, int len) {
        return !!std::atoi(std::string(s, len).c_str());
    }
};

template <>
class RedisValTrans<std::string> {
   public:
    std::string operator()(const char *s, int len) {
        if (len == 0) return "";
        return std::string(s, len);
    }
};

class RedisResult {
   private:
    RedisStatus m_status;
    bool m_parse_finished;

   protected:
    std::string m_err_desc;

   protected:
    void parse_error(const char *s, int64_t len) {
        if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
            m_err_desc = std::string(s, 1, len - 3);
            set_parse_finished(true);
            set_status(RedisStatus::REPLY_ERROR);
        }
    }

    template <typename T>
    void parse_simple_string(const char *s, int64_t len, T &data) {
        if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
            data = RedisValTrans<T>()(s + 1, len - 3);
            set_parse_finished(true);
            set_status(RedisStatus::OK);
        }
    }

    template <typename T>
    void parse_digit(const char *s, int64_t len, T &data) {
        if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
            data = RedisValTrans<T>()(s + 1, len - 3);
            set_parse_finished(true);
            set_status(RedisStatus::OK);
        }
    }

    template <typename T>
    void parse_string(const char *s, int64_t len, T &data) {
        int digit = std::atoi(s + 1);
        if (digit == -1 && std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
            m_err_desc = "nil";
            set_parse_finished(true);
            set_status(RedisStatus::NIL_ERROR);
        } else {
            if (len - digit - 2 == 1 + 1 + (int64_t)floor(log10(digit)) + 2) {
                data = RedisValTrans<T>()(s + (len - digit - 2), digit);
                set_parse_finished(true);
                set_status(RedisStatus::OK);
            } else {
                set_parse_finished(false);
            }
        }
    }

    template <typename T>
    void parse_vector(const char *s, int64_t len, int64_t &has_parse, std::vector<T> &data) {
        int64_t rst_size = std::atoi(s + 1);
        if (rst_size <= 0) {
            if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
                has_parse = len;
                set_parse_finished(true);
                set_status(RedisStatus::OK);
            }
            return;
        }
        if (has_parse == 0) {
            has_parse += (1 + 1 + (int64_t)floor(log10(rst_size)) + 2);
        }
        while (has_parse < len) {
            int item_len = std::atoi(s + has_parse + 1);
            if (item_len == -1) {
                has_parse += (1 + 2 + 2);
                if (has_parse > len) {
                    has_parse -= (1 + 2 + 2);
                    return;
                }
                data.push_back(RedisValTrans<T>()(s + has_parse, 0));
            } else {
                has_parse += (1 + 1 + (int64_t)floor(log10(item_len)) + 2);
                if (has_parse + item_len + 2 > len) {
                    has_parse -= (1 + 1 + (int64_t)floor(log10(item_len)) + 2);
                    return;
                }
                data.push_back(RedisValTrans<T>()(s + has_parse, item_len));
                has_parse += (item_len + 2);
            }
        }
        if (rst_size == (int64_t)data.size()) {
            set_parse_finished(true);
            set_status(RedisStatus::OK);
        }
    }

   public:
    typedef std::shared_ptr<RedisResult> ptr;

    RedisResult() : m_status(RedisStatus::INVALID), m_parse_finished(false), m_err_desc("") {}
    virtual ~RedisResult(){};

   public:
    void set_status(RedisStatus s) { m_status = s; };
    void set_err_desc(const std::string err) { m_err_desc = err; }
    void set_parse_finished(bool v) { m_parse_finished = true; }

    RedisStatus get_status() const { return m_status; }
    std::string &get_err_desc() { return m_err_desc; }

    bool parse_finished() const { return m_parse_finished; }
    bool is_ok() { return m_status == RedisStatus::OK; }
    bool is_parse_error() { return m_status == RedisStatus::PARSE_ERROR; }

   public:
    virtual void parse(const char *s, int64_t len) = 0;
};

template <typename T>
class RedisResultVal : public RedisResult {
   protected:
    T m_data;

   public:
    typedef std::shared_ptr<RedisResultVal<T>> ptr;

    RedisResultVal()
        : RedisResult() {}

   public:
    T &get_data() { return m_data; }

   public:
    void parse(const char *s, int64_t len) override {
        if (len < 2) return;
        switch (s[0]) {
            case '-': {
                parse_error(s, len);
                break;
            }
            case '+': {
                parse_simple_string(s, len, m_data);
                break;
            }
            case ':': {
                parse_digit(s, len, m_data);
                break;
            }
            case '$': {
                parse_string(s, len, m_data);
                break;
            }
            default:
                m_err_desc = std::string(s, len);
                set_parse_finished(true);
                set_status(RedisStatus::PARSE_ERROR);
                break;
        }
    }
};

template <typename T>
class RedisResultVector : public RedisResult {
   private:
    int64_t m_has_parse;
    std::vector<T> m_data;

   public:
    typedef std::shared_ptr<RedisResultVector<T>> ptr;

    RedisResultVector()
        : RedisResult(), m_has_parse(0) {}

   public:
    std::vector<T> &get_data() { return m_data; }

   public:
    void parse(const char *s, int64_t len) override {
        if (len < 2) return;
        switch (s[0]) {
            case '-': {
                parse_error(s, len);
                break;
            }
            case '*': {
                parse_vector(s, len, m_has_parse, m_data);
                break;
            }
            default:
                m_err_desc = std::string(s, len);
                set_parse_finished(true);
                set_status(RedisStatus::PARSE_ERROR);
                break;
        }
    }
};

template <typename T>
class RedisResultScan : public RedisResult {
   private:
    int64_t m_has_parse;
    bool init_first;
    size_t second_offset;
    std::pair<int64_t, std::vector<T>> m_data;

   public:
    typedef std::shared_ptr<RedisResultScan<T>> ptr;

    RedisResultScan()
        : RedisResult(), m_has_parse(0), init_first(false), second_offset(0) {}

   public:
    std::pair<int64_t, std::vector<T>> &get_data() { return m_data; }

   public:
    void parse(const char *s, int64_t len) override {
        if (len < 2) return;
        switch (s[0]) {
            case '-': {
                parse_error(s, len);
                break;
            }
            case '*': {
                int64_t rst_size = std::atoi(s + 1);
                if (rst_size != 2) {
                    if (std::string(s, len - 2, 2) == TIGER_REDIS_CRLF) {
                        set_parse_finished(true);
                        set_status(RedisStatus::PARSE_ERROR);
                    }
                    break;
                }
                if (!init_first) {
                    int64_t first_start = 1 + 1 + (size_t)log10(rst_size) + 2;
                    if (first_start >= len) break;
                    int64_t digit = std::atol(s + first_start + 1);
                    m_has_parse += first_start + 1 + (int64_t)floor(log10(digit)) + 2;
                    if (m_has_parse + digit + 2 > len) {
                        m_has_parse -= first_start + 1 + (int64_t)floor(log10(digit)) + 2;
                        break;
                    }
                    init_first = true;
                    m_data.first = std::atol(s + m_has_parse);
                    m_has_parse += digit + 2;
                    second_offset = m_has_parse;
                }
                std::cout << "m_has_parse:" << std::string(s + second_offset, len - second_offset) << std::endl;
                parse_vector<T>(s + second_offset, len - second_offset, m_has_parse, m_data.second);
                std::cout << "m_has_parse:" << m_has_parse << " size:" << m_data.second.size() << std::endl;
                break;
            }
            default:
                m_err_desc = std::string(s, len);
                set_parse_finished(true);
                set_status(RedisStatus::PARSE_ERROR);
                break;
        }
    }
};

}  // namespace redis

}  // namespace tiger

#endif