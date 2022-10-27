/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_servlet.h"

#include <fnmatch.h>

namespace tiger {

namespace http {

Servlet::Servlet(const std::string &name)
    : m_name(name) {
}

NotFoundServlet::NotFoundServlet()
    : Servlet("NotFoundServlet") {
}

int32_t NotFoundServlet::handle(HTTPRequest::ptr request,
                                HTTPResponse::ptr response,
                                HTTPSession::ptr session) {
    static const std::string &rsp_body =
        "<html>\
            <head>\
                <title>\
                    404 Not Found\
                </title>\
            </head>\
            <body>\
                <center>\
                    <h1>404 Not Found</h1>\
                </center>\
            </body>\
        </html>";
    response->set_status(HTTPStatus::NOT_FOUND);
    response->set_header("Content-Type", "text/html");
    response->set_header("Server", "tigerkin/1.0.0");
    response->set_body(rsp_body);
    return 0;
}

FunctionServlet::FunctionServlet(Callback cb)
    : Servlet("FunctionServlet"), m_cb(cb) {
}

int32_t FunctionServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
    return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch()
    : Servlet("ServletDispatch") {
    m_default_servlet.reset(new NotFoundServlet);
}

int32_t ServletDispatch::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
    Servlet::ptr slt = get_matched_servlet(request->get_path());
    if (slt) {
        slt->handle(request, response, session);
    }
    return 0;
}

void ServletDispatch::add_servlet(const std::string &path, Servlet::ptr slt) {
    ReadWriteLock::WriteLock lock(m_lock);
    m_datas[path] = slt;
}

void ServletDispatch::add_servlet(const std::string &path, FunctionServlet::Callback cb) {
    ReadWriteLock::WriteLock lock(m_lock);
    m_datas[path].reset(new FunctionServlet(cb));
}

void ServletDispatch::add_glob_servlet(const std::string &path, Servlet::ptr slt) {
    ReadWriteLock::WriteLock lock(m_lock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            m_globs.erase(it);
        }
    }
    m_globs.push_back(std::make_pair(path, slt));
}

void ServletDispatch::add_glob_servlet(const std::string &path, FunctionServlet::Callback cb) {
    add_glob_servlet(path, FunctionServlet::ptr(new FunctionServlet(cb)));
}

void ServletDispatch::del_servlet(const std::string &path) {
    ReadWriteLock::WriteLock lock(m_lock);
    m_datas.erase(path);
}

void ServletDispatch::del_glob_servlet(const std::string &path) {
    ReadWriteLock::WriteLock lock(m_lock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            m_globs.erase(it);
            break;
        }
    }
}

void ServletDispatch::set_default_servlet(Servlet::ptr default_servlet) {
    m_default_servlet = default_servlet;
}

Servlet::ptr ServletDispatch::get_default_servlet() {
    return m_default_servlet;
}

Servlet::ptr ServletDispatch::get_servlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_lock);
    std::unordered_map<std::string, Servlet::ptr>::iterator it = m_datas.find(path);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::get_glob_servlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_lock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::get_matched_servlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_lock);
    std::unordered_map<std::string, Servlet::ptr>::iterator mIt = m_datas.find(path);
    if (mIt != m_datas.end()) {
        return mIt->second;
    }
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator gIt = m_globs.begin();
    while (gIt != m_globs.end()) {
        if (!fnmatch(gIt->first.c_str(), path.c_str(), 0)) {
            return gIt->second;
        }
    }
    return m_default_servlet;
}

}  // namespace http
}  // namespace tiger