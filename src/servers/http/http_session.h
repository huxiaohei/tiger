/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/10/25
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_HTTP_SESSION_H__
#define __TIGER_HTTP_HTTP_SESSION_H__

#include "../../streams/socket_stream.h"
#include "http_parser.h"

namespace tiger {

namespace http {

class HTTPSession : public SocketStream {
   public:
    typedef std::shared_ptr<HTTPSession> ptr;

    HTTPSession(Socket::ptr socket, bool owner = true);
    HTTPRequest::ptr recv_request();
    ssize_t send_response(HTTPResponse::ptr rsp);
};

}  // namespace http

}  // namespace tiger

#endif