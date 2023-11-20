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

#include "address.h"

namespace tiger
{

class URI
{
  private:
    std::string m_scheme;
    std::string m_user;
    std::string m_host;
    int32_t m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

  public:
    typedef std::shared_ptr<URI> ptr;

    URI() : m_port(0){};
    static URI::ptr Create(const std::string &uri_str);

  public:
    const std::string &get_scheme() const
    {
        return m_scheme;
    }
    const std::string &get_user() const
    {
        return m_user;
    }
    const std::string &get_host() const
    {
        return m_host;
    }
    const std::string &get_path() const
    {
        static std::string s_default_path = "/";
        return m_path.empty() ? s_default_path : m_path;
    }
    const std::string &get_query() const
    {
        return m_query;
    }
    const std::string &get_fragment() const
    {
        return m_fragment;
    }
    int32_t get_port() const;

    bool is_default_port() const;

  public:
    Address::ptr create_address() const;
    std::string to_string() const;

  public:
    friend std::ostream &operator<<(std::ostream &os, const URI &uri);
    friend std::ostream &operator<<(std::ostream &os, const URI::ptr uri);
};

} // namespace tiger

#endif