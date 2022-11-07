/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "ws_server.h"

namespace tiger {

namespace http {

WSServer::WSServer(IOManager::ptr worker, IOManager::ptr accept, std::string const &name)
    : TCPServer(worker, accept, name) {
    m_dispatch = std::make_shared<WSServletDispatch>("Websocket Dispatch");
}

void WSServer::handle_client(Socket::ptr client) {
    auto session = std::make_shared<WSSession>(client);
    do {
        HTTPRequest::ptr header = session->handle_shake();
        if (!header) {
            TIGER_LOG_I(SYSTEM_LOG) << "[client handshake error]";
            break;
        }
        auto servlet = m_dispatch->get_ws_servlet(header->get_path());
        if (!servlet) {
            TIGER_LOG_I(SYSTEM_LOG) << "[no match servlet]";
            break;
        }
        int rt = servlet->on_connect(header, session);
        if (rt) {
            TIGER_LOG_I(SYSTEM_LOG) << "[Websocket connect error [rt:" << rt << "]]";
            break;
        }
        while (true) {
            auto msg = session->recv_message();
            if (!msg) break;
            TIGER_LOG_D(SYSTEM_LOG) << "[Websocket recv messgae [" << msg->get_data() << "]]";
            rt = servlet->handle(header, msg, session);
            if (rt) {
                TIGER_LOG_D(SYSTEM_LOG) << "[handle rt:" << rt << "]";
                break;
            }
        }
        servlet->on_close(header, session);
    } while (false);
    session->close();
}

}  // namespace http

}  // namespace tiger
