/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_MACRO_H__
#define __TIGER_MACRO_H__

#include <assert.h>
#include <string.h>

#include "log.h"

#if defined __GNUC__ || defined __llvm__
#define TIGER_LIKELY(x) __builtin_expect(!!(x), 1)
#define TIGER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define TIGER_LIKELY(x) (x)
#define TIGER_UNLIKELY(x) (x)
#endif

#define TIGER_LOG_D(name) TIGER_LOG_DEBUG(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name))
#define TIGER_LOG_I(name) TIGER_LOG_INFO(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name))
#define TIGER_LOG_W(name) TIGER_LOG_WARN(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name))
#define TIGER_LOG_E(name) TIGER_LOG_ERROR(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name))

#define TIGER_LOG_FMT_D(name, fmt, ...) TIGER_LOG_FMT_DEBUG(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name), fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_I(name, fmt, ...) TIGER_LOG_FMT_INFO(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name), fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_W(name, fmt, ...) TIGER_LOG_FMT_WARN(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name), fmt, __VA_ARGS__)
#define TIGER_LOG_FMT_E(name, fmt, ...) TIGER_LOG_FMT_ERROR(tiger::SingletonLoggerMgr::Instance()->get_logger_by(name), fmt, __VA_ARGS__)

#endif
