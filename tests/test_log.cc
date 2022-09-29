/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tiger.h"

void test_log_base() {
    const std::string LOG_NAME = "BASE";
    auto logger = std::make_shared<tiger::Logger>(LOG_NAME);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    logger->add_appender(std_out_appender);
    auto event = std::make_shared<tiger::LogEvent>(
        logger, tiger::LogLevel::DEBUG, __FILE__, __LINE__, 0, 0, time(0), "MAIN");
    auto w = tiger::LogEventWarp(event);
    w.ss() << "Hello Tiger Log Base";

    auto event_format = std::make_shared<tiger::LogEvent>(
        logger, tiger::LogLevel::DEBUG, __FILE__, __LINE__, 0, 0, time(0), "MAIN");
    int a = 30;
    event_format->format("%d, %p", a, &a);
    auto w1 = tiger::LogEventWarp(event_format);
}

void test_log_macro() {
    const std::string LOG_NAME = "MACRO";
    auto logger = std::make_shared<tiger::Logger>(LOG_NAME);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    logger->add_appender(std_out_appender);
    TIGER_LOG_LEVEL(logger, tiger::LogLevel::DEBUG) << "Hello Tiger Log Macro";
    int a = 50;
    TIGER_LOG_FMT_LEVEL(logger, tiger::LogLevel::DEBUG, "%d, %p", a, &a);
}

void test_log_macro_simple() {
    const std::string LOG_NAME = "MACRO SIMPLE";
    auto logger = std::make_shared<tiger::Logger>(LOG_NAME);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    logger->add_appender(std_out_appender);
    TIGER_LOG_DEBUG(logger) << "Hello debug";
    TIGER_LOG_INFO(logger) << "Hello info";
    TIGER_LOG_WARN(logger) << "Hello warn";
    TIGER_LOG_ERROR(logger) << "Hello error";

    int a = 100;
    TIGER_LOG_FMT_DEBUG(logger, "%d, %p", a, &a);
    TIGER_LOG_FMT_INFO(logger, "%d, %p", a, &a);
    TIGER_LOG_FMT_WARN(logger, "%d, %p", a, &a);
    TIGER_LOG_FMT_ERROR(logger, "%d, %p", a, &a);
}

void test_log_mgr_base() {
    const std::string MGR = "MGR";
    const std::string ROOT = "ROOT";
    const std::string ERROR_NAME = "ERROR_NAME";

    auto logger = std::make_shared<tiger::Logger>(MGR);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    logger->add_appender(std_out_appender);
    tiger::SingletonLoggerMgr::Instance()->add_logger(logger);

    TIGER_LOG_D(ROOT) << "Log manager `ROOT` debug level";
    TIGER_LOG_I(MGR) << "Log manager `MGR` logger info level";
    TIGER_LOG_W(MGR) << "Log manager `MGR` logger warn level";
    TIGER_LOG_E(ROOT) << "Log manager `ROOT` logger error level";

    TIGER_LOG_E(ERROR_NAME) << "Log manager name error";
}

void test_log_level() {
    const std::string DEBUG_LEVEL = "DEBUG_LEVEL";
    const std::string ERROR_LEVEL = "ERROR_LEVEL";

    auto debug_logger = std::make_shared<tiger::Logger>(DEBUG_LEVEL, tiger::LogLevel::DEBUG);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto std_out_appender = std::make_shared<tiger::StdOutLogAppender>(
        formatter, tiger::LogLevel::DEBUG);
    debug_logger->add_appender(std_out_appender);

    auto error_logger = std::make_shared<tiger::Logger>(ERROR_LEVEL, tiger::LogLevel::ERROR);
    error_logger->add_appender(std_out_appender);

    tiger::SingletonLoggerMgr::Instance()->add_logger(debug_logger);
    tiger::SingletonLoggerMgr::Instance()->add_logger(error_logger);

    TIGER_LOG_D(DEBUG_LEVEL) << "TEST_LOG_LEVEL_DEBUG_LEVEL_DEBUG";
    TIGER_LOG_I(DEBUG_LEVEL) << "TEST_LOG_LEVEL_DEBUG_LEVEL_INFO";
    TIGER_LOG_W(DEBUG_LEVEL) << "TEST_LOG_LEVEL_DEBUG_LEVEL_WARN";
    TIGER_LOG_E(DEBUG_LEVEL) << "TEST_LOG_LEVEL_DEBUG_LEVEL_ERROR";

    TIGER_LOG_D(ERROR_LEVEL) << "TEST_LOG_LEVEL_ERROR_LEVEL_DEBUG";
    TIGER_LOG_I(ERROR_LEVEL) << "TEST_LOG_LEVEL_ERROR_LEVEL_INFO";
    TIGER_LOG_W(ERROR_LEVEL) << "TEST_LOG_LEVEL_ERROR_LEVEL_WARN";
    TIGER_LOG_E(ERROR_LEVEL) << "TEST_LOG_LEVEL_ERROR_LEVEL_ERROR";
}

void test_log_file() {
    const std::string LOG_NAME = "FILE";
    auto logger = std::make_shared<tiger::Logger>(LOG_NAME);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto file_logger_appender = std::make_shared<tiger::FileLogAppender>(
        formatter, tiger::LogLevel::DEBUG, "file_test", 5);
    logger->add_appender(file_logger_appender);
    tiger::SingletonLoggerMgr::Instance()->add_logger(logger);

    TIGER_LOG_D(LOG_NAME) << "TEST_LOG_FILE_DEBUG";
    TIGER_LOG_I(LOG_NAME) << "TEST_LOG_FILE_INFO";
    TIGER_LOG_W(LOG_NAME) << "TEST_LOG_FILE_WARN";
    TIGER_LOG_E(LOG_NAME) << "TEST_LOG_FILE_ERROR";

    int a = 100;
    TIGER_LOG_FMT_D(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_D", a, &a);
    TIGER_LOG_FMT_I(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_I", a, &a);
    TIGER_LOG_FMT_W(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_W", a, &a);
    TIGER_LOG_FMT_E(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_E", a, &a);
}

void test_log_from_yaml() {
    if (tiger::SingletonLoggerMgr::Instance()->add_loggers("log", "../conf/log.yml")) {
        TIGER_LOG_I(tiger::SYSTEM_LOG) << "init success";
    } else {
        TIGER_LOG_E(tiger::SYSTEM_LOG) << "init fail";
    }
}

void test_log_performance() {
    const std::string LOG_NAME = "PERFORMANCE";
    auto logger = std::make_shared<tiger::Logger>(LOG_NAME);
    auto formatter = std::make_shared<tiger::LogFormatter>();
    auto file_logger_appender = std::make_shared<tiger::FileLogAppender>(
        formatter, tiger::LogLevel::DEBUG, "performance", 3600);
    logger->add_appender(file_logger_appender);
    tiger::SingletonLoggerMgr::Instance()->add_logger(logger);

    auto t = tiger::Millisecond();
    int a = 100;
    for (auto i = 0; i < 1000000; ++i) {
        TIGER_LOG_D(LOG_NAME) << "TEST_LOG_FILE_DEBUG";
        TIGER_LOG_I(LOG_NAME) << "TEST_LOG_FILE_INFO";
        TIGER_LOG_W(LOG_NAME) << "TEST_LOG_FILE_WARN";
        TIGER_LOG_E(LOG_NAME) << "TEST_LOG_FILE_ERROR";

        TIGER_LOG_FMT_D(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_D", a, &a);
        TIGER_LOG_FMT_I(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_I", a, &a);
        TIGER_LOG_FMT_W(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_W", a, &a);
        TIGER_LOG_FMT_E(LOG_NAME, "%s %d %p", "TIGER_LOG_FMT_E", a, &a);
    }
    TIGER_LOG_D(LOG_NAME) << "PERFORMANCE COST TIME: " << tiger::Millisecond() - t << "ms";
}

void test_log_thread() {
    auto t = tiger::Millisecond();
    std::vector<tiger::Thread::ptr> v;
    for (size_t i = 0; i < 3; ++i) {
        v.push_back(std::make_shared<tiger::Thread>([]() {
            for (size_t i = 0; i < 1000000; ++i) {
                TIGER_LOG_D(tiger::TEST_LOG) << "LOG " << i;
            }
        },
                                                    "LogThread"));
    }
    for (size_t i = 0; i < v.size(); ++i) {
        v[i]->join();
    }
    TIGER_LOG_D(tiger::TEST_LOG) << "thread log cost time: " << tiger::Millisecond() - t << "ms";
}

int main() {
    test_log_base();
    test_log_macro();
    test_log_macro_simple();
    test_log_level();
    test_log_mgr_base();
    test_log_file();
    test_log_from_yaml();
    test_log_performance();
    test_log_thread();
    return 0;
}
