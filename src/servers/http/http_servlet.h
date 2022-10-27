/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_SERVLET_H__
#define __TIGER_HTTP_SERVLET_H__

#include <functional>
#include <unordered_map>

#include "../../mutex.h"
#include "http.h"
#include "http_session.h"

namespace tiger {

namespace http {

class Servlet {
   protected:
    std::string m_name;

   public:
    typedef std::shared_ptr<Servlet> ptr;
    typedef std::function<int32_t(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session)> Callback;

    Servlet(const std::string &name="");
    virtual ~Servlet(){};

   public:
    const std::string &get_name() { return m_name; }

   public:
    virtual int32_t handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) = 0;
};

class FunctionServlet : public Servlet {
   private:
    Callback m_cb;

   public:
    typedef std::shared_ptr<FunctionServlet> ptr;

    FunctionServlet(Callback cb);

   public:
    int32_t handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;
};

class NotFoundServlet : public Servlet {
   public:
    typedef std::shared_ptr<NotFoundServlet> ptr;

    NotFoundServlet();

   public:
    int32_t handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;
};

class ServletDispatch : public Servlet {
   private:
    ReadWriteLock m_lock;
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
    Servlet::ptr m_default_servlet;

   public:
    typedef std::shared_ptr<ServletDispatch> ptr;

    ServletDispatch();

   public:
    int32_t handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;

   public:
    void add_servlet(const std::string &path, Servlet::ptr slt);
    void add_servlet(const std::string &path, FunctionServlet::Callback cb);
    void add_glob_servlet(const std::string &path, Servlet::ptr slt);
    void add_glob_servlet(const std::string &path, FunctionServlet::Callback cb);

    Servlet::ptr get_default_servlet();
    Servlet::ptr get_servlet(const std::string &path);
    Servlet::ptr get_glob_servlet(const std::string &path);
    Servlet::ptr get_matched_servlet(const std::string &path);

    void del_servlet(const std::string &path);
    void del_glob_servlet(const std::string &path);

    void set_default_servlet(Servlet::ptr defaultServlet);
};

}  // namespace http
}  // namespace tiger

#endif