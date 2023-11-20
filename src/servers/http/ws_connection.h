/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/03
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_WS_CONNECTION_H__
#define __TIGER_HTTP_WS_CONNECTION_H__

#include "http_connection.h"
#include "ws_session.h"

namespace tiger
{

namespace http
{

class WSConnection : public HTTPConnection
{
  public:
    typedef std::shared_ptr<WSConnection> ptr;

    WSConnection(Socket::ptr socket, bool owner = true);

  public:
    static std::pair<HTTPResult::ptr, WSConnection::ptr> Create(URI::ptr uri, uint64_t timeout_ms = 60000,
                                                                const std::map<std::string, std::string> &headers = {});
    static std::pair<HTTPResult::ptr, WSConnection::ptr> Create(const std::string &url, uint64_t timeout_ms = 60000,
                                                                const std::map<std::string, std::string> &headers = {});

  public:
    WSFrameMessage::ptr recv_message();
    int32_t send_message(WSFrameMessage::ptr msg, bool fin = true);
    int32_t send_message(const std::string &msg, int32_t opcode = WSFrameHead::TEXT_FRAME, bool fin = true);

    int32_t ping();
    int32_t pong();
};

} // namespace http

} // namespace tiger

#endif