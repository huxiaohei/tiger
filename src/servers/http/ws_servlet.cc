/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/04
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "ws_servlet.h"

namespace tiger
{

namespace http
{

FunctionWSServlet::FunctionWSServlet(HandleCallback cb, ConnectCallback connect_cb, CloseCallback close_cb,
                                     const std::string &name)
    : WSServlet(name), m_handle(cb), m_connect(connect_cb), m_close(close_cb)
{
}

int32_t FunctionWSServlet::on_connect(HTTPRequest::ptr header, WSSession::ptr session)
{
    if (m_connect)
    {
        m_connect(header, session);
    }
    return 0;
}

int32_t FunctionWSServlet::on_close(HTTPRequest::ptr header, WSSession::ptr session)
{
    if (m_close)
    {
        m_close(header, session);
    }
    return 0;
}

int32_t FunctionWSServlet::handle(HTTPRequest::ptr header, WSFrameMessage::ptr msg, WSSession::ptr session)
{
    if (m_handle)
    {
        m_handle(header, msg, session);
    }
    return 0;
}

WSServletDispatch::WSServletDispatch(const std::string &name) : ServletDispatch(name)
{
}

void WSServletDispatch::add_servlet(const std::string &url, FunctionWSServlet::HandleCallback cb,
                                    FunctionWSServlet::ConnectCallback connect_cb,
                                    FunctionWSServlet::CloseCallback close_cb)
{
    ServletDispatch::add_servlet(url, std::make_shared<FunctionWSServlet>(cb, connect_cb, close_cb));
}

void WSServletDispatch::add_servlet(const std::string &url, WSServlet::ptr servlet)
{
    ServletDispatch::add_servlet(url, servlet);
}

void WSServletDispatch::add_glob_servlet(const std::string &url, FunctionWSServlet::HandleCallback cb,
                                         FunctionWSServlet::ConnectCallback connect_cb,
                                         FunctionWSServlet::CloseCallback close_cb)
{
    ServletDispatch::add_glob_servlet(url, std::make_shared<FunctionWSServlet>(cb, connect_cb, close_cb));
}

void WSServletDispatch::add_glob_servlet(const std::string &url, WSServlet::ptr servlet)
{
    ServletDispatch::add_glob_servlet(url, servlet);
}

WSServlet::ptr WSServletDispatch::get_ws_servlet(const std::string &url)
{
    auto slt = get_matched_servlet(url);
    return std::dynamic_pointer_cast<WSServlet>(slt);
}

} // namespace http

} // namespace tiger