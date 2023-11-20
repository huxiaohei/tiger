/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_LOG_H__
#define __TIGER_LOG_H__

#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "coroutine.h"
#include "mutex.h"
#include "singleton.h"
#include "thread.h"
#include "util.h"

#define TIGER_LOG_LEVEL(logger, level)                                                                                 \
    tiger::LogEventWarp(std::make_shared<tiger::LogEvent>(                                                             \
                            logger, level, __FILE__, __LINE__, tiger::Thread::CurThreadId(),                           \
                            tiger::Coroutine::CurCoroutineId(), tiger::Millisecond(), tiger::Thread::CurThreadName())) \
        .ss()

#define TIGER_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                                   \
    tiger::LogEventWarp(std::make_shared<tiger::LogEvent>(                                                             \
                            logger, level, __FILE__, __LINE__, tiger::Thread::CurThreadId(),                           \
                            tiger::Coroutine::CurCoroutineId(), tiger::Millisecond(), tiger::Thread::CurThreadName())) \
        .event()                                                                                                       \
        ->format(fmt, __VA_ARGS__);

#define TIGER_LOG_DEBUG(logger) TIGER_LOG_LEVEL(logger, tiger::LogLevel::DEBUG)
#define TIGER_LOG_INFO(logger) TIGER_LOG_LEVEL(logger, tiger::LogLevel::INFO)
#define TIGER_LOG_WARN(logger) TIGER_LOG_LEVEL(logger, tiger::LogLevel::WARN)
#define TIGER_LOG_ERROR(logger) TIGER_LOG_LEVEL(logger, tiger::LogLevel::ERROR)

#define TIGER_LOG_FMT_DEBUG(logger, fmt, ...) TIGER_LOG_FMT_LEVEL(logger, tiger::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_INFO(logger, fmt, ...) TIGER_LOG_FMT_LEVEL(logger, tiger::LogLevel::INFO, fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_WARN(logger, fmt, ...) TIGER_LOG_FMT_LEVEL(logger, tiger::LogLevel::WARN, fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_ERROR(logger, fmt, ...) TIGER_LOG_FMT_LEVEL(logger, tiger::LogLevel::ERROR, fmt, __VA_ARGS__)

namespace tiger
{

class Logger;

class LogLevel
{
  public:
    enum Level
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4
    };
    static const std::string ToString(LogLevel::Level level);
    static const LogLevel::Level FromString(const std::string &level);
};

typedef struct
{
    std::string type;
    LogLevel::Level level;
    std::string file;
    int interval;
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[" << type << "," << LogLevel::ToString(level) << "," << file << "," << interval << "]";
        return ss.str();
    }
} AppenderDefine;

typedef struct
{
    std::string name;
    LogLevel::Level level;
    std::string formater;
    std::vector<AppenderDefine> appenders;
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[\n\tname:" << name << "\n\tlevel:" << level << "\n\tformater:" << formater << "\n\tappenders:";
        for (auto it : appenders)
        {
            ss << "\n\t\t" << it.to_string();
        }
        ss << "\n]";
        return ss.str();
    }
} LoggerDefine;

class LogEvent
{
  private:
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    const char *m_file;
    int32_t m_line = 0;
    uint32_t m_thread_id = 0;
    uint64_t m_co_id = 0;
    time_t m_time;
    std::string m_thread_name;
    std::stringstream m_ss;

  public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t thread_id,
             uint64_t co_id, time_t time, const std::string &thread_name)
        : m_logger(logger), m_level(level), m_file(file), m_line(line), m_thread_id(thread_id), m_co_id(co_id),
          m_time(time), m_thread_name(thread_name){};

    const std::shared_ptr<Logger> logger()
    {
        return m_logger;
    }
    LogLevel::Level level()
    {
        return m_level;
    }
    const char *file()
    {
        return m_file;
    }
    int32_t line()
    {
        return m_line;
    }
    uint32_t thread_id()
    {
        return m_thread_id;
    }
    uint64_t co_id()
    {
        return m_co_id;
    }
    time_t time()
    {
        return m_time;
    }
    const std::string &thread_name()
    {
        return m_thread_name;
    }
    std::stringstream &ss()
    {
        return m_ss;
    }

  public:
    void format(const char *fmt, ...);
    void format(const char *fmt, va_list al);
};

class LogEventWarp
{
  private:
    LogEvent::ptr m_event;

  public:
    LogEventWarp(LogEvent::ptr event) : m_event(event){};
    ~LogEventWarp();

    LogEvent::ptr event()
    {
        return m_event;
    }
    std::stringstream &ss()
    {
        return m_event->ss();
    }
};

class LogFormatter
{
  public:
    class FormatItem
    {
      protected:
        std::string m_fmt;

      public:
        typedef std::shared_ptr<FormatItem> ptr;

        FormatItem(const std::string &fmt = "") : m_fmt(fmt){};
        virtual ~FormatItem(){};
        virtual void format(const std::shared_ptr<Logger> logger, std::ostream &os, LogLevel::Level level,
                            LogEvent::ptr event) = 0;
    };

  private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;

    void init();

  public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string &pattern = "");
    ~LogFormatter(){};

    const std::string &pattern()
    {
        return m_pattern;
    }

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream &format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, LogEvent::ptr event);
};

class LogAppender
{
  protected:
    LogFormatter::ptr m_formatter;
    LogLevel::Level m_level;
    SpinLock m_lock;

  public:
    typedef std::shared_ptr<LogAppender> ptr;

    LogAppender(LogFormatter::ptr formatter, LogLevel::Level level) : m_formatter(formatter), m_level(level)
    {
    }
    virtual ~LogAppender(){};

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
};

class StdOutLogAppender : public LogAppender
{
  public:
    typedef std::shared_ptr<StdOutLogAppender> ptr;

    StdOutLogAppender(LogFormatter::ptr formatter, LogLevel::Level level) : LogAppender(formatter, level){};

    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
};

class FileLogAppender : public LogAppender
{
  private:
    std::string m_path;
    size_t m_interval;
    time_t m_start_time;
    time_t m_end_time;
    std::ofstream m_filestream;

  private:
    bool open_file();

  public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(LogFormatter::ptr formatter, LogLevel::Level level, const std::string &path,
                    size_t interval = 7200);

    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
};

class Logger : public std::enable_shared_from_this<Logger>
{
  private:
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenders;
    SpinLock m_lock;

  public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string &name = "ROOT", LogLevel::Level level = LogLevel::DEBUG) : m_name(name), m_level(level){};

    const std::string &name() const
    {
        return m_name;
    }
    const LogLevel::Level level() const
    {
        return m_level;
    }

  public:
    void add_appender(LogAppender::ptr appender);
    void add_appenders(std::vector<LogAppender::ptr> appenders);
    void del_appender(LogAppender::ptr appender);
    void debug(const LogEvent::ptr event);
    void info(const LogEvent::ptr event);
    void warn(const LogEvent::ptr event);
    void error(const LogEvent::ptr event);
    void log(LogLevel::Level level, const LogEvent::ptr event);
};

class LoggerMgr
{
  private:
    std::unordered_map<std::string, Logger::ptr> m_logger_map;
    SpinLock m_lock;

  public:
    LoggerMgr();

    bool add_loggers(const std::string &name, const std::string &path);
    bool add_logger(Logger::ptr logger);
    bool add_logger(const LoggerDefine &log_def);
    bool del_logger(const std::string &logger_name);
    Logger::ptr get_logger_by(const std::string &logger_name = "ROOT");
};

typedef Singleton<LoggerMgr> SingletonLoggerMgr;

} // namespace tiger

#endif