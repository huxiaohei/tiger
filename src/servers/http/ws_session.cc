/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/02
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "ws_session.h"

#include "../../endian.h"
#include "../../hash.h"

namespace tiger
{

namespace http
{

static tiger::ConfigVar<uint32_t>::ptr g_websocket_message_max_size = tiger::Config::Lookup<uint32_t>(
    "tiger.websocket.msgMaxSize", (uint32_t)(1024 * 1024 * 32), "websocket message max size");

std::string WSFrameHead::to_string() const
{
    std::stringstream ss;
    ss << "[WSFrameHead"
       << " opcode:" << opcode << " rsv3:" << rsv3 << " rsv2:" << rsv2 << " rsv1:" << rsv1 << " fin:" << fin
       << " playload:" << payload << " mask:" << mask << "]";
    return ss.str();
}

WSFrameMessage::WSFrameMessage(int opcode, const std::string &data) : m_opcode(opcode), m_data(data)
{
}

WSSession::WSSession(Socket::ptr socket, bool owner) : HTTPSession(socket, owner)
{
}

HTTPRequest::ptr WSSession::handle_shake()
{
    HTTPRequest::ptr req;
    do
    {
        req = recv_request();
        if (!req)
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket shake fail invalid http request]";
            break;
        }
        if (strcasecmp(req->get_header("Upgrade").c_str(), "websocket"))
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket header error"
                                    << " Upgrade:" << req->get_header("Upgrade") << "]";
            break;
        }
        if (strcasecmp(req->get_header("Connection").c_str(), "Upgrade"))
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket header error"
                                    << " Connection:" << req->get_header("Connection") << "]";
            break;
        }
        if (req->get_header_as<int>("Sec-WebSocket-Version") != 13)
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket header error"
                                    << " Sec-WebSocket-Version:" << req->get_header_as<int>("Sec-WebSocket-Version")
                                    << "]";
            break;
        }
        std::string key = req->get_header("Sec-WebSocket-Key");
        if (key.empty())
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket header Sec-webSocket-Version is empty]";
            break;
        }
        std::string v = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        v = tiger::Base64Encode(tiger::SHA1(v));
        req->set_websocket(true);
        auto rsp = std::make_shared<HTTPResponse>(req->get_version(), req->is_close());
        rsp->set_status(HTTPStatus::SWITCHING_PROTOCOLS);
        rsp->set_websocket(true);
        rsp->set_reason("Websocket Protocol Handshake");
        rsp->set_header("Upgrade", "WebSocket");
        rsp->set_header("Connection", "Upgrade");
        rsp->set_header("Sec-WebSocket-Accept", v);
        send_response(rsp);
        TIGER_LOG_I(SYSTEM_LOG) << "[websocket success request:" << req << "]";
        TIGER_LOG_I(SYSTEM_LOG) << "[websocket success response:" << rsp << "]";
        return req;
    } while (false);
    if (req)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[websocket fail request:" << req << "]";
    }
    return nullptr;
}

WSFrameMessage::ptr WSSession::recv_message()
{
    int opcode = 0;
    std::string data;
    int cur_len = 0;
    do
    {
        WSFrameHead ws_head;
        if (read_fixed_size(&ws_head, sizeof(ws_head)) <= 0)
            break;
        if (ws_head.opcode == WSFrameHead::PING)
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket receive ping msg]";
            if (pong() <= 0)
                break;
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket send pong msg]";
        }
        else if (ws_head.opcode == WSFrameHead::PONG)
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket receive pong msg]";
        }
        else if (ws_head.opcode == WSFrameHead::CONTINUE || ws_head.opcode == WSFrameHead::TEXT_FRAME ||
                 ws_head.opcode == WSFrameHead::BIN_FRAME)
        {
            if (!ws_head.mask)
            {
                TIGER_LOG_E(SYSTEM_LOG) << "[websocket mask error mask:" << ws_head.mask << "]";
                break;
            }
            uint64_t length = 0;
            if (ws_head.payload == 126)
            {
                uint16_t len = 0;
                if (read_fixed_size(&len, sizeof(len)) <= 0)
                    break;
                length = tiger::bswap_on_little_endian(len);
            }
            else if (ws_head.payload == 127)
            {
                uint64_t len = 0;
                if (read_fixed_size(&len, sizeof(len)) <= 0)
                    break;
                length = tiger::bswap_on_little_endian(len);
            }
            else
            {
                length = ws_head.payload;
            }
            if ((cur_len + length) >= g_websocket_message_max_size->val())
            {
                TIGER_LOG_E(SYSTEM_LOG) << "[wensocket message is too bigger "
                                        << " size:" << (cur_len + length)
                                        << " maxSize:" << g_websocket_message_max_size->val() << "]";
                break;
            }
            char mask[4] = {0};
            if (ws_head.mask)
            {
                if (read_fixed_size(mask, sizeof(mask)) <= 0)
                    break;
            }
            data.resize(cur_len + length);
            if (read_fixed_size(&data[cur_len], length) <= 0)
                break;
            if (ws_head.mask)
            {
                for (int i = 0; i < (int)length; ++i)
                {
                    data[cur_len + i] ^= mask[i % 4];
                }
            }
            cur_len += length;
            if (!opcode && ws_head.opcode != WSFrameHead::CONTINUE)
                opcode = ws_head.opcode;
            if (ws_head.fin)
            {
                TIGER_LOG_D(SYSTEM_LOG) << "[websocket handshake finished"
                                        << " data:" << data << "]";
                return std::make_shared<WSFrameMessage>(opcode, std::move(data));
            }
        }
        else if (ws_head.opcode == WSFrameHead::CLOSE)
        {
            TIGER_LOG_D(SYSTEM_LOG) << "[websocket closed remoteAddr:" << get_remote_address() << "]";
            break;
        }
        else
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket ivalid opcode"
                                    << " opcode:" << opcode << "]";
        }
    } while (true);
    close();
    return nullptr;
}

int32_t WSSession::send_message(WSFrameMessage::ptr msg, bool fin)
{
    do
    {
        WSFrameHead ws_head;
        memset(&ws_head, 0, sizeof(ws_head));
        ws_head.fin = fin;
        ws_head.opcode = msg->get_opcode();
        ws_head.mask = false;
        uint64_t size = msg->get_data().size();
        if (size < 126)
        {
            ws_head.payload = size;
        }
        else if (size < 65536)
        {
            ws_head.payload = 126;
        }
        else
        {
            ws_head.payload = 127;
        }
        if (write_fixed_size(&ws_head, sizeof(ws_head)) <= 0)
            break;
        if (ws_head.payload == 126)
        {
            uint16_t len = size;
            len = tiger::bswap_on_little_endian(len);
            if (write_fixed_size(&len, sizeof(len)) <= 0)
                break;
        }
        else if (ws_head.payload == 127)
        {
            uint64_t len = tiger::bswap_on_big_endian(size);
            if (write_fixed_size(&len, sizeof(len)) <= 0)
                break;
        }
        if (write_fixed_size(msg->get_data().c_str(), size) <= 0)
            break;
        return size + sizeof(ws_head);
    } while (false);
    close();
    return -1;
}

int32_t WSSession::send_message(const std::string &msg, int32_t opcode, bool fin)
{
    return send_message(std::make_shared<WSFrameMessage>(opcode, msg), fin);
}

int32_t WSSession::ping()
{
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::PING;
    int32_t v = write_fixed_size(&ws_head, sizeof(ws_head));
    if (v < 0)
    {
        close();
    }
    return v;
}

int32_t WSSession::pong()
{
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::PONG;
    int32_t v = write_fixed_size(&ws_head, sizeof(ws_head));
    if (v <= 0)
    {
        close();
    }
    return v;
}

} // namespace http
} // namespace tiger
