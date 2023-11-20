/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_WS_SERVER_H__
#define __TIGER_HTTP_WS_SERVER_H__

#include "../tcp_server.h"
#include "ws_servlet.h"

namespace tiger
{

namespace http
{

class WSServer : public TCPServer
{
  private:
    WSServletDispatch::ptr m_dispatch;

  public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(IOManager::ptr worker = IOManager::GetThreadIOM(), IOManager::ptr accept = IOManager::GetThreadIOM(),
             std::string const &name = "");

  protected:
    virtual void handle_client(Socket::ptr client) override;

  public:
    WSServletDispatch::ptr get_servlet_dispatch() const
    {
        return m_dispatch;
    }
    void set_servlet_dispatch(WSServletDispatch::ptr dispatch)
    {
        m_dispatch = dispatch;
    }
};

} // namespace http

} // namespace tiger

#endif