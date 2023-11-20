/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_session.h"

namespace tiger
{

namespace http
{

HTTPSession::HTTPSession(Socket::ptr socket, bool owner) : SocketStream(socket, owner)
{
}

HTTPRequest::ptr HTTPSession::recv_request()
{
    auto parser = std::make_shared<HTTPRequestParser>();
    uint64_t buff_size = HTTPRequestParser::GetHTTPRequestBufferSize();
    std::shared_ptr<char> buffer(new char[buff_size], [](char *ptr) { delete[] ptr; });
    char *data = buffer.get();
    int offset = 0;
    do
    {
        int len = read(data + offset, buff_size - offset);
        if (len < 0)
        {
            close();
            return nullptr;
        }
        len += offset;
        size_t nparse = parser->execute(data, len);
        if (parser->has_error())
        {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (TIGER_UNLIKELY(offset == (int)buff_size))
        {
            close();
            TIGER_LOG_E(SYSTEM_LOG) << "[buffer bugger than max buffer]";
            return nullptr;
        }
        if (parser->is_finished())
        {
            break;
        }
    } while (true);
    int64_t length = parser->get_content_length();
    if (length > 0)
    {
        std::string body;
        body.resize(length);

        int len = 0;
        if (length >= offset)
        {
            memcpy(&body[0], data, offset);
            len = offset;
        }
        else
        {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        if (length > 0)
        {
            if (read_fixed_size(&body[len], length) <= 0)
            {
                close();
                return nullptr;
            }
        }
        parser->get_data()->set_body(body);
    }
    parser->get_data()->init();
    return parser->get_data();
}

ssize_t HTTPSession::send_response(HTTPResponse::ptr rsp)
{
    std::stringstream ss;
    ss << rsp;
    std::string data = ss.str();
    return write_fixed_size(data.c_str(), data.size());
}

} // namespace http

} // namespace tiger
