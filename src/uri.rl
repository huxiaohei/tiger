/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/01
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "uri.h"

namespace tiger {

%%{
    machine uri_parser;

    action marku {
        mark = fpc;
    }

    action markh {
        mark = fpc;
    }

    action save_scheme {
        uri->m_scheme = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    action save_port {
        if (fpc != mark)
            uri->m_port = atoi(mark);
        mark = nullptr;
    }

    action save_user {
        if (mark)
            uri->m_user = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    action save_host {
        if (mark)
            uri->m_host = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    action save_path {
        if (mark)
            uri->m_path = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    action save_query {
        if (mark)
            uri->m_query = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    action save_fragment {
        if (mark)
            uri->m_fragment = std::string(mark, fpc - mark);
        mark = nullptr;
    }

    gen_delims = ":" | "/" | "?" | "#" | "[" | "]" | "@";
    sub_delims = "!" | "$" | "&" | "'" | "(" | ")" | "*" | "+" | "," | ";" | "=";
    reserved = gen_delims | sub_delims;
    unreserved = alpha | digit | "-" | "." | "_" | "~";
    pct_encoded = "%" xdigit xdigit;
    
    scheme = (alpha (alpha | digit | "+" | "-" | ".")*) > marku %save_scheme;
    user = (unreserved | pct_encoded | sub_delims | ":")*;
    dec_octet = digit | [1-9] digit | 1 digit{2} | 2 [0-4] digit | "25" [0-5];
    ip_v4 = dec_octet "." dec_octet "." dec_octet "." dec_octet;
    h16 = xdigit{1,4};
    ls32 = (h16 ":" h16) | ip_v4;
    ip_v6 = (                         (h16 ":"){6} ls32) |
            (                    "::" (h16 ":"){5} ls32) |
            ((             h16)? "::" (h16 ":"){4} ls32) |
            (((h16 ":"){1} h16)? "::" (h16 ":"){3} ls32) |
            (((h16 ":"){2} h16)? "::" (h16 ":"){2} ls32) |
            (((h16 ":"){3} h16)? "::" (h16 ":"){1} ls32) |
            (((h16 ":"){4} h16)? "::"              ls32) |
            (((h16 ":"){5} h16)? "::"              h16 ) |
            (((h16 ":"){6} h16)? "::"                  );
    ip_future = "v" xdigit+ "." (unreserved | sub_delims | ":")+;
    ip_literal = "[" (ip_v6 | ip_future) "]";
    reg_name = (unreserved | pct_encoded | sub_delims)*;
    host = ip_literal | ip_v4 | reg_name;
    port = digit*;
    authority = ( (user %save_user "@")? host >markh %save_host (":" port >markh %save_port)? ) > markh;
    pchar = ( (any -- ascii) | unreserved | pct_encoded | sub_delims | ":" | "@");
    segment = pchar*;
    segment_nz = pchar+;
    segment_nz_nc = (pchar - ":")+;
    path_abempty = (("/" segment))? ("/" segment)*;
    path_absolute = ("/" (segment_nz ("/" segment)*)?);
    path_noscheme = segment_nz_nc ("/" segment)*;
    path_rootless = segment_nz ("/" segment)*;
    path_empty = "";
    path = (path_abempty | path_absolute | path_noscheme | path_rootless | path_empty);
    query = (pchar | "/" | "?")* >marku %save_query;
    fragment = (pchar | "/" | "?")* >marku %save_fragment;
    hier_part = ("//" authority path_abempty > markh %save_path) | path_absolute | path_rootless | path_empty;
    relative_part = ("//" authority path_abempty) | path_absolute | path_noscheme | path_empty;
    relative_ref = relative_part ( "?" query )? ( "#" fragment )?;
    uri = scheme ":" hier_part ( "?" query )? ( "#" fragment )?;
    uri_reference = uri | relative_ref;
    main := uri_reference;

    write data;
}%%

URI::ptr URI::Create(const std::string& uri_str) {
    auto uri = std::make_shared<URI>();
    int cs = 0;
    %% write init;
    const char *mark = 0;
    const char *p = uri_str.c_str();
    const char *pe = p + uri_str.size();
    const char *eof = pe;
    %% write exec;
    if (cs == uri_parser_error) {
        return nullptr;
    } else if (cs >= uri_parser_first_final) {
        return uri;
    }
    return nullptr;
}

Address::ptr URI::create_address() const {
    auto addr = IPAddress::LookupAny(m_host);
    if (addr) {
        addr->set_port(get_port());
    }
    return addr;
}

int32_t URI::get_port() const {
    if (m_port) return m_port;
    if (m_scheme == "wss" || m_scheme == "https") return 443;
    if (m_scheme == "ws" || m_scheme == "http") return 80;
    return m_port;
}

bool URI::is_default_port() const {
    if (m_port == 0) return true;
    if (m_scheme == "wss" || m_scheme == "https") return m_port == 80;
    if (m_scheme == "ws" || m_scheme == "http") return m_port == 443;
    return false;
}

std::ostream& operator<<(std::ostream& os, const URI& uri) {
    os << uri.m_scheme << "://"
       << uri.m_user << (uri.m_user.empty() ? "" : "@")
       << uri.m_host << (uri.is_default_port() ? "" : (":" + std::to_string(uri.m_port)))
       << uri.m_path << (uri.m_query.empty() ? "" : "?") << uri.m_query
       << (uri.m_fragment.empty() ? "" : "#") << uri.m_fragment;
    return os;
}

std::ostream& operator<<(std::ostream& os, const URI::ptr uri) {
    os << uri->m_scheme << "://"
       << uri->m_user << (uri->m_user.empty() ? "" : "@")
       << uri->m_host << (uri->is_default_port() ? "" : (":" + std::to_string(uri->m_port)))
       << uri->m_path << (uri->m_query.empty() ? "" : "?") << uri->m_query
       << (uri->m_fragment.empty() ? "" : "#") << uri->m_fragment;
    return os;
}

std::string URI::to_string() const {
    std::stringstream ss;
    ss << this;
    return ss.str();
}

}  // namespace tiger