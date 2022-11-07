/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_SERVER_H__
#define __TIGER_HTTP_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_servlet.h"
#include "http_session.h"

namespace tiger {

namespace http {

class HTTPServer : public TCPServer {
   private:
    bool m_keepalive;
    ServletDispatch::ptr m_dispatch;

   public:
    typedef std::shared_ptr<HTTPServer> ptr;

    HTTPServer(bool keepalive = false,
               IOManager::ptr worker = IOManager::GetThreadIOM(),
               IOManager::ptr accept = IOManager::GetThreadIOM(),
               std::string const &name = "");

   public:
    ServletDispatch::ptr get_servlet_dispatch() const { return m_dispatch; }
    void set_servlet_dispatch(ServletDispatch::ptr v) { m_dispatch = v; }

   protected:
    void handle_client(Socket::ptr client) override;
};

}  // namespace http
}  // namespace tiger

#endif