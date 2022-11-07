/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_WS_SERVLET_H__
#define __TIGER_HTTP_WS_SERVLET_H__

#include <functional>

#include "http_servlet.h"
#include "ws_session.h"

namespace tiger {

namespace http {

class WSServlet : public Servlet {
   public:
    typedef std::shared_ptr<WSServlet> ptr;

    WSServlet(const std::string &name = "")
        : Servlet(name) {}

    virtual ~WSServlet() {}

    virtual int32_t handle(HTTPRequest::ptr header,
                           HTTPResponse::ptr response,
                           HTTPSession::ptr session) override {
        return 0;
    }

    virtual int32_t on_connect(HTTPRequest::ptr header,
                               WSSession::ptr session) = 0;
    virtual int32_t on_close(HTTPRequest::ptr header,
                             WSSession::ptr session) = 0;
    virtual int32_t handle(HTTPRequest::ptr header,
                           WSFrameMessage::ptr msg,
                           WSSession::ptr session) = 0;
};

class FunctionWSServlet : public WSServlet {
   public:
    typedef std::shared_ptr<FunctionWSServlet> ptr;

    typedef std::function<int32_t(HTTPRequest::ptr header, WSSession::ptr session)> ConnectCallback;
    typedef std::function<int32_t(HTTPRequest::ptr header, WSSession::ptr session)> CloseCallback;
    typedef std::function<int32_t(HTTPRequest::ptr header, WSFrameMessage::ptr msg, WSSession::ptr session)> HandleCallback;

    FunctionWSServlet(HandleCallback cb,
                      ConnectCallback connect_cb = nullptr,
                      CloseCallback close_cb = nullptr,
                      const std::string &name = "");

   protected:
    HandleCallback m_handle;
    ConnectCallback m_connect;
    CloseCallback m_close;

   public:
    virtual int32_t on_connect(HTTPRequest::ptr header,
                               WSSession::ptr session) override;
    virtual int32_t on_close(HTTPRequest::ptr header,
                             WSSession::ptr session) override;
    virtual int32_t handle(HTTPRequest::ptr header,
                           WSFrameMessage::ptr msg,
                           WSSession::ptr session) override;
};

class WSServletDispatch : public ServletDispatch {
   public:
    typedef std::shared_ptr<WSServletDispatch> ptr;

    WSServletDispatch(const std::string &name = "");

   public:
    void add_servlet(const std::string &url,
                     FunctionWSServlet::HandleCallback cb,
                     FunctionWSServlet::ConnectCallback connect_cb,
                     FunctionWSServlet::CloseCallback close_cb);
    void add_servlet(const std::string &url,
                     WSServlet::ptr servlet);
    void add_glob_servlet(const std::string &url,
                          FunctionWSServlet::HandleCallback cb,
                          FunctionWSServlet::ConnectCallback connect_cb,
                          FunctionWSServlet::CloseCallback close_cb);
    void add_glob_servlet(const std::string &url,
                          WSServlet::ptr servlet);

    WSServlet::ptr get_ws_servlet(const std::string &url);
};

}  // namespace http

}  // namespace tiger

#endif