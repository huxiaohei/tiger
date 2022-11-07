/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_server.h"

namespace tiger {

namespace http {

HTTPServer::HTTPServer(bool keepalive,
                       IOManager::ptr worker,
                       IOManager::ptr accept,
                       const std::string &name)
    : TCPServer(worker, accept, name),
      m_keepalive(keepalive),
      m_dispatch(std::make_shared<ServletDispatch>()) {
}

void HTTPServer::handle_client(Socket::ptr client) {
    auto session = std::make_shared<HTTPSession>(client);
    do {
        auto req = session->recv_request();
        if (!req) {
            if (errno == ETIMEDOUT) {
                TIGER_LOG_I(SYSTEM_LOG) << "[connection timeout "
                                        << " localAddr:" << client->get_local_addr()
                                        << " remoteAddr:" << client->get_remote_addr() << "]";

            } else {
                TIGER_LOG_W(SYSTEM_LOG) << "[recv fail"
                                        << " localAddr:" << client->get_local_addr()
                                        << " remoteAddr:" << client->get_remote_addr()
                                        << " errno:" << strerror(errno) << "]";
            }
            break;
        }
        auto rsp = std::make_shared<HTTPResponse>(req->get_version(), req->is_close() || !m_keepalive);
        m_dispatch->handle(req, rsp, session);
        session->send_response(rsp);
        if (!m_keepalive || req->is_close()) {
            break;
        }
    } while (m_keepalive);
    session->close();
}

}  // namespace http
}  // namespace tiger