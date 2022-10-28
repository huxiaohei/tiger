/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_UTIL_H__
#define __TIGER_UTIL_H__

#include <chrono>
#include <iostream>
#include <string>

namespace tiger {

bool IsValidName(const std::string &name);

time_t Millisecond();
time_t Second();
std::string Time2Str(time_t ts, const std::string &format);

class StringUtils {
   public:
    static std::string Format(const char *fmt, ...);
    static std::string Formatv(const char *fmt, va_list ap);

    static std::string UrlEncode(const std::string &str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

    static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");
    static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");
    static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

    static std::string WStringToString(const std::wstring &w);
    static std::wstring StringToWString(const std::string &s);
};

class FSUtils {
   public:
    static bool Unlink(const std::string &filename, bool exist = false);
    static const std::string ReadFile(const std::string &filename);
};

}  // namespace tiger

#endif