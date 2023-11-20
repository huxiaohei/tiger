/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "util.h"

#include <execinfo.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "macro.h"

namespace tiger
{

bool IsValidName(const std::string &name)
{
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789.") == std::string::npos)
        return true;
    return false;
}

time_t Millisecond()
{
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

time_t Second()
{
    auto tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

std::string Time2Str(time_t ts, const std::string &format)
{
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

std::string StringUtils::Random(size_t len, const std::string chars)
{
    if (len == 0 || chars.empty())
        return "";
    std::string rt;
    rt.resize(len);
    for (size_t i = 0, cnt = chars.size(); i < len; ++i)
    {
        rt[i] = chars[rand() % cnt];
    }
    return rt;
}

std::string StringUtils::Format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    auto v = Formatv(fmt, ap);
    va_end(ap);
    return v;
}

std::string StringUtils::Formatv(const char *fmt, va_list ap)
{
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, ap);
    if (len == -1)
    {
        return "";
    }
    std::string ret(buf, len);
    free(buf);
    return ret;
}

static const char uri_chars[256] = {
    /* 0 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    /* 64 */
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    /* 128 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 192 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const char xdigit_chars[256] = {
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

std::string StringUtils::UrlEncode(const std::string &str, bool space_as_plus)
{
    static const char *hexdigits = "0123456789ABCDEF";
    std::string *ss = nullptr;
    const char *end = str.c_str() + str.length();
    for (const char *c = str.c_str(); c < end; ++c)
    {
        if (!CHAR_IS_UNRESERVED(*c))
        {
            if (!ss)
            {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if (*c == ' ' && space_as_plus)
            {
                ss->append(1, '+');
            }
            else
            {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        }
        else if (ss)
        {
            ss->append(1, *c);
        }
    }
    if (!ss)
    {
        return str;
    }
    else
    {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtils::UrlDecode(const std::string &str, bool space_as_plus)
{
    std::string *ss = nullptr;
    const char *end = str.c_str() + str.length();
    for (const char *c = str.c_str(); c < end; ++c)
    {
        if (*c == '+' && space_as_plus)
        {
            if (!ss)
            {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        }
        else if (*c == '%' && (c + 2) < end && isxdigit(*(c + 1)) && isdigit(*(c + 2)))
        {
            if (!ss)
            {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
            c += 2;
        }
        else if (ss)
        {
            ss->append(1, *c);
        }
    }
    if (!ss)
    {
        return str;
    }
    else
    {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtils::Trim(const std::string &str, const std::string &delimit)
{
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos)
    {
        return "";
    }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

std::string StringUtils::TrimLeft(const std::string &str, const std::string &delimit)
{
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos)
    {
        return "";
    }
    return str.substr(begin);
}

std::string StringUtils::TrimRight(const std::string &str, const std::string &delimit)
{
    auto end = str.find_last_not_of(delimit);
    if (end == std::string::npos)
    {
        return "";
    }
    return str.substr(0, end);
}

std::string StringUtils::WStringToString(const std::wstring &w)
{
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t *wchSrc = w.c_str();
    size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
    char *chDest = new char[nDestSize];
    memset(chDest, 0, nDestSize);
    wcstombs(chDest, wchSrc, nDestSize);
    std::string strResult = chDest;
    delete[] chDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
}

std::wstring StringUtils::StringToWString(const std::string &s)
{
    std::string strLocle = setlocale(LC_ALL, "");
    const char *chSrc = s.c_str();
    size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
    wchar_t *wchDest = new wchar_t[nDestSize];
    wmemset(wchDest, 0, nDestSize);
    mbstowcs(wchDest, chSrc, nDestSize);
    std::wstring wstrResult = wchDest;
    delete[] wchDest;
    setlocale(LC_ALL, strLocle.c_str());
    return wstrResult;
}

static int __lstat(const char *file, struct stat *st = nullptr)
{
    struct stat lst;
    int rst = lstat(file, &lst);
    if (st)
    {
        *st = lst;
    }
    return rst;
}

bool FSUtils::Unlink(const std::string &filename, bool exist)
{
    if (!exist && __lstat(filename.c_str()))
        ;
    return ::unlink(filename.c_str()) == 0;
}

const std::string FSUtils::ReadFile(const std::string &filename)
{
    std::ifstream file;
    std::stringstream ss;
    file.open(filename, std::ios_base::in);
    if (!file.is_open())
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[file not exist"
                                << " file:" << filename << "]";
        return ss.str();
    }
    ss << file.rdbuf();
    file.close();
    return ss.str();
}

} // namespace tiger
