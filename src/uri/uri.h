/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/18
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

/**
 * 参考标准
 * https://www.ietf.org/rfc/rfc3986.txt
 *
 *   foo://user@example.com:8042/over/there?name=ferret#nose
 *   \_/   \___________________/\_________/ \_________/ \__/
 *    |           |                  |            |       |
 *  scheme    authority             path        query   fragment
 *    |   ___________________________|_____
 *   / \ /                                 \
 *   urn:example:over:there:name:ferret:nose
 *
 * authority: [user@host:port]
 */

#ifndef __TIGER_URI_H__
#define __TIGER_URI_H__

#include <stdlib.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace tiger {

class Uri {
   private:
    std::string m_scheme;
    std::string m_user;
    std::string m_host;
    int32_t m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

   public:
    typedef std::shared_ptr<Uri> ptr;

    Uri() : m_port(0){};
    static Uri::ptr Create(const std::string &uri_str);

    const std::string &scheme() const { return m_scheme; }
    const std::string &user() const { return m_user; }
    const std::string &host() const { return m_host; }
    const std::string &path() const { return m_host; }
    const std::string &query() const { return m_query; }
    const std::string &fragment() const { return m_fragment; }
    int32_t port() const;

    bool is_default_port() const;
    friend std::ostream &operator<<(std::ostream &os, const Uri &uri);
    friend std::ostream &operator<<(std::ostream &os, const Uri::ptr uri);
    std::string to_string() const;
};

}  // namespace tiger

#endif