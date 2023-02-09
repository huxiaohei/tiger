/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2023/02/08
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_REDIS_REIDS_H__
#define __TIGER_REDIS_REIDS_H__

#include <memory>
#include <string>

namespace tiger {

namespace redis {

#define CRLF "\r\n"

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

class RedisResult {
   public:
    typedef std::shared_ptr<RedisResult> ptr;

    RedisResult() : m_status(RedisStatus::INVALID), m_parse_finished(false) {}
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

   private:
    RedisStatus m_status;
    bool m_parse_finished;
};

class RedisResultStr : public RedisResult {
   public:
    typedef std::shared_ptr<RedisResultStr> ptr;

   public:
    std::string &get_data() { return m_data; };

   public:
    void parse(const char *s, int len) override;

   private:
    std::string m_data;
};

}  // namespace redis

}  // namespace tiger

#endif