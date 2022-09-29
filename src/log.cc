/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "log.h"

#include <stdarg.h>

#include "config.h"

namespace tiger {

template <>
class LexicalCast<std::string, LoggerDefine> {
   public:
    LoggerDefine operator()(const std::string &str) {
        YAML::Node root = YAML::Load(str);
        LoggerDefine v;
        v.name = root["name"].as<std::string>();
        v.level = LogLevel::FromString(root["level"].as<std::string>());
        v.formater = root["formater"].as<std::string>();
        for (size_t i = 0; i < root["appenders"].size(); ++i) {
            auto it = root["appenders"][i];
            AppenderDefine appender;
            appender.type = it["type"].as<std::string>();
            appender.level = LogLevel::FromString(it["level"].as<std::string>());
            if (appender.type == "FileLogAppender") {
                appender.file = it["file"].as<std::string>();
                appender.interval = it["interval"].as<int>();
            }
            v.appenders.push_back(appender);
        }
        return v;
    }
};

template <>
class LexicalCast<LoggerDefine, std::string> {
   public:
    std::string operator()(const LoggerDefine &v) {
        YAML::Node node;
        node["name"] = v.name;
        node["level"] = LogLevel::ToString(v.level);
        node["formater"] = v.formater;
        for (size_t i = 0; i < v.appenders.size(); ++i) {
            YAML::Node sub_node;
            sub_node["type"] = v.appenders[i].type;
            sub_node["level"] = LogLevel::ToString(v.appenders[i].level);
            if (v.appenders[i].type == "FileLogAppender") {
                sub_node["file"] = v.appenders[i].file;
                sub_node["interval"] = v.appenders[i].interval;
            }
            node["appenders"].push_back(sub_node);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

const std::string LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name)         \
    case LogLevel::name: \
        return #name

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
#undef XX
        default:
            return "UNKNOW";
    }
}

const LogLevel::Level LogLevel::FromString(const std::string &level) {
#define XX(name) \
    if (#name == level) return Level::name

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
#undef XX
    return Level::UNKNOW;
}

void LogEvent::format(const char *fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char *fmt, va_list al) {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogEventWarp::~LogEventWarp() {
    m_event->logger()->log(m_event->level(), m_event);
};

class StringFormatItem : public LogFormatter::FormatItem {
   public:
    StringFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << m_fmt;
    }
};

class MessageFormatItem : public LogFormatter::FormatItem {
   public:
    MessageFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->ss().str();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
   public:
    LevelFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << LogLevel::ToString(level);
    }
};

class FileNameFormatItem : public LogFormatter::FormatItem {
   public:
    FileNameFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->file();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadIdFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->thread_id();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
   public:
    NewLineFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << "\n";
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
   public:
    DateTimeFormatItem(const std::string &fmt = "%Y-%m-%d %H:%M:%S")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        struct tm tm;
        time_t time = event->time();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_fmt.c_str(), &tm);
        os << buf;
    }
};

class LoggerNameFormatItem : public LogFormatter::FormatItem {
   public:
    LoggerNameFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << logger->name();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
   public:
    LineFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->line();
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
   public:
    TabFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << "\t";
    }
};

class CoIdFormatItem : public LogFormatter::FormatItem {
   public:
    CoIdFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->co_id();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadNameFormatItem(const std::string &fmt = "")
        : LogFormatter::FormatItem(fmt) {}
    void format(const std::shared_ptr<Logger> logger, std::ostream &os,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->thread_name();
    }
};

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {
    if (m_pattern.empty()) {
        m_pattern = "%d{%Y-%m-%d %H:%M:%S} %t %N %F [%p] [%c] %f:%l %m%n";
    }
    init();
}

void LogFormatter::init() {
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // 如果输入两个%% 则在日志中输入一个%
        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                ++i;
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;
        while (n < m_pattern.size()) {
            if (!fmt_status && (!isalpha(m_pattern[n]) &&
                                m_pattern[n] != '{' &&
                                m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - "
                      << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {

#define XX(tag, Format)                                                            \
    {                                                                              \
#tag, [](const std::string &fmt) { return std::make_shared<Format>(fmt); } \
    }

        XX(m, MessageFormatItem),     // m:消息
        XX(p, LevelFormatItem),       // p:日志级别
        XX(c, LoggerNameFormatItem),  // c:日志名称
        XX(t, ThreadIdFormatItem),    // t:线程id
        XX(n, NewLineFormatItem),     // n:换行
        XX(d, DateTimeFormatItem),    // d:时间
        XX(f, FileNameFormatItem),    // f:文件名
        XX(l, LineFormatItem),        // l:行号
        XX(T, TabFormatItem),         // T:Tab
        XX(F, CoIdFormatItem),        // F:协程id
        XX(N, ThreadNameFormatItem),  // N:线程名称
#undef XX
    };

    for (auto &i : vec) {
        if (std::get<2>(i) == 0) {
            FormatItem::ptr items = std::make_shared<StringFormatItem>(std::get<0>(i));
            m_items.push_back(items);
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                continue;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,
                                 LogLevel::Level level,
                                 LogEvent::ptr event) {
    std::stringstream ss;
    for (auto &i : m_items)
        i->format(logger, ss, level, event);
    return ss.str();
}

std::ostream &LogFormatter::format(std::shared_ptr<Logger> logger,
                                   std::ostream &ofs,
                                   LogLevel::Level level,
                                   LogEvent::ptr event) {
    for (auto &i : m_items)
        i->format(logger, ofs, level, event);
    return ofs;
}

void StdOutLogAppender::log(std::shared_ptr<Logger> logger,
                            LogLevel::Level level,
                            LogEvent::ptr event) {
    if (level >= m_level) {
#ifdef __TIGER_MULTI_THREAD__
        SpinLock::Lock lock(m_lock);
#endif
        std::cout << m_formatter->format(logger, level, event);
    }
}

FileLogAppender::FileLogAppender(LogFormatter::ptr formatter,
                                 LogLevel::Level level,
                                 const std::string &path,
                                 size_t interval)
    : LogAppender(formatter, level), m_path(path), m_interval(interval) {
    m_start_time = Second();
    m_start_time -= (m_start_time % 3600);
    m_interval = m_interval < 3600 ? 3600 : m_interval / 3600 * 3600;
#ifdef __TIGER_MULTI_THREAD__
    SpinLock::Lock lock(m_lock);
#endif
    open_file();
}

bool FileLogAppender::open_file() {
    if (!!m_filestream) {
        m_filestream.close();
    }
    struct tm tm;
    m_start_time = Second();
    m_end_time = m_start_time + m_interval;
    const std::string fmt = "%Y_%m_%d_%H";
    localtime_r(&m_start_time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), fmt.c_str(), &tm);
    std::string tmp_path = m_path;
    tmp_path += ".log.";
    tmp_path += buf;
    m_filestream.open(tmp_path, std::ofstream::app);
    return !!m_filestream;
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        time_t now = Second();
        if (TIGER_UNLIKELY(now > m_end_time)) {
            m_start_time = now;
#ifdef __TIGER_MULTI_THREAD__
            SpinLock::Lock lock(m_lock);
#endif
            open_file();
        }
#ifdef __TIGER_MULTI_THREAD__
        SpinLock::Lock lock(m_lock);
#endif
        m_formatter->format(logger, m_filestream, level, event);
    }
}

void Logger::add_appender(LogAppender::ptr appender) {
#ifdef __TIGER_MULTI_THREAD__
    SpinLock::Lock lock(m_lock);
#endif
    m_appenders.push_back(appender);
}

void Logger::add_appenders(std::vector<LogAppender::ptr> appenders) {
    for (auto it : appenders)
        add_appender(it);
}

void Logger::del_appender(LogAppender::ptr appender) {
#ifdef __TIGER_MULTI_THREAD__
    SpinLock::Lock lock(m_lock);
#endif
    auto it = m_appenders.begin();
    while (it != m_appenders.end()) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
        ++it;
    }
}

void Logger::debug(const LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(const LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(const LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(const LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::log(LogLevel::Level level,
                 const LogEvent::ptr event) {
    if (level < m_level) return;
#ifdef __TIGER_MULTI_THREAD__
    SpinLock::Lock lock(m_lock);
#endif
    auto self = shared_from_this();
    for (auto &it : m_appenders)
        it->log(self, level, event);
}

LoggerMgr::LoggerMgr() {
    auto logger = std::make_shared<tiger::Logger>("ROOT");
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    logger->add_appender(std_out_appender);
    add_logger(logger);
}

bool LoggerMgr::add_loggers(const std::string &name, const std::string &path) {
    ConfigVar<std::vector<LoggerDefine>>::ptr cfg = Config::Lookup(name + ".log", (std::vector<LoggerDefine>){}, "__logs__");
    Config::LoadFromYmal(name, path);
    bool suc = true;
    for (auto &it : cfg->val()) {
        suc = add_logger(it) && suc;
    }
    return suc;
}

bool LoggerMgr::add_logger(const LoggerDefine &def) {
    auto logger = std::make_shared<tiger::Logger>(def.name, def.level);
    auto formatter = std::make_shared<tiger::LogFormatter>(def.formater);
    for (auto &it : def.appenders) {
        if (it.type == "FileLogAppender") {
            auto appender = std::make_shared<FileLogAppender>(formatter, it.level, it.file);
            logger->add_appender(appender);
        } else {
            auto appender = std::make_shared<StdOutLogAppender>(formatter, it.level);
            logger->add_appender(appender);
        }
    }
    return add_logger(logger);
}

bool LoggerMgr::add_logger(Logger::ptr logger) {
    if (m_logger_map.find(logger->name()) != m_logger_map.end()) {
        TIGER_LOG_W(SYSTEM_LOG) << logger->name() << " logger has exist";
        return false;
    }
    m_logger_map.insert(std::pair<std::string, Logger::ptr>(logger->name(), logger));
    return true;
}

bool LoggerMgr::del_logger(const std::string &logger_name) {
    if (logger_name.empty()) return false;
    auto it = m_logger_map.find(logger_name);
    if (it == m_logger_map.end()) return false;
    m_logger_map.erase(it->first);
    return true;
}

Logger::ptr LoggerMgr::get_logger_by(const std::string &logger_name) {
#ifdef __TIGER_MULTI_THREAD__
    SpinLock::Lock lock(m_lock);
#endif
    auto it = m_logger_map.find(logger_name);
    if (it == m_logger_map.end()) {
        auto tmp = m_logger_map.at("ROOT");
        return tmp;
    }
    return it->second;
}

}  // namespace tiger