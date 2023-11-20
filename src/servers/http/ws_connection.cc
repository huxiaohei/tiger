/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/03
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "ws_connection.h"

#include <iostream>

#include "../../endian.h"
#include "../../hash.h"

namespace tiger
{

namespace http
{

static tiger::ConfigVar<uint32_t>::ptr g_websocket_message_max_size = tiger::Config::Lookup<uint32_t>(
    "tiger.websocket.msgMaxSize", (uint32_t)(1024 * 1024 * 32), "websocket message max size");
static tiger::ConfigVar<std::string>::ptr g_websocket_sec_key =
    tiger::Config::Lookup<std::string>("tiger.websocket.secKey", "abcdefghijklmnop", "websocket sec key");

WSConnection::WSConnection(Socket::ptr socket, bool owner) : HTTPConnection(socket, owner)
{
}

std::pair<HTTPResult::ptr, WSConnection::ptr> WSConnection::Create(URI::ptr uri, uint64_t timeout_ms,
                                                                   const std::map<std::string, std::string> &headers)
{
    auto addr = uri->create_address();
    if (!addr)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[Create fail"
                                << " addr:" << addr << "]";
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::INVALID_HOST,
                                                           "invalid host:" + uri->get_host()),
                              nullptr);
    }
    Socket::ptr socket = Socket::CreateTCP(addr);
    if (!socket)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[Craete fail"
                                << " errno:" << std::strerror(errno) << "]";
        return std::make_pair(
            std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::CREATE_SOCKET_ERROR, "create socket error"),
            nullptr);
    }
    if (!socket->connect(addr))
    {
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::CONNECT_FAIL,
                                                           "connect fail addr:" + addr->to_string()),
                              nullptr);
    }
    socket->set_recv_timeout(timeout_ms);
    auto conn = std::make_shared<WSConnection>(socket);
    auto req = std::make_shared<HTTPRequest>();
    req->set_path(uri->get_path());
    req->set_query(uri->get_query());
    req->set_fragment(uri->get_fragment());
    req->set_method(HTTPMethod::GET);
    bool has_host = false;
    bool has_conn = false;
    for (auto &it : headers)
    {
        if (strcasecmp(it.first.c_str(), "Connection") == 0)
        {
            has_conn = true;
        }
        else if (!has_host && strcasecmp(it.first.c_str(), "Host") == 0)
        {
            has_host = !it.second.empty();
        }
        req->set_header(it.first, it.second);
    }
    req->set_websocket(true);
    if (!has_conn)
    {
        req->set_header("Connection", "Upgrade");
    }
    req->set_header("Upgrade", "websocket");
    req->set_header("Sec-WebSocket-Version", "13");
    req->set_header("Sec-WebSocket-Key", tiger::Base64Decode(g_websocket_sec_key->val()));
    if (!has_host)
    {
        req->set_header("Host", uri->get_host());
    }
    int rt = conn->send_request(req);
    if (rt == 0)
    {
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::SEND_CLOSE_BY_PEER,
                                                           "send request closed by peer addr:" + addr->to_string()),
                              nullptr);
    }
    if (rt < 0)
    {
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::SEND_SOCKET_ERROR,
                                                           "send request socket error addr:" + addr->to_string()),
                              nullptr);
    }
    auto rsp = conn->recv_response();
    if (!rsp)
    {
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::TIMEOUT,
                                                           "recv response timeout addr:" + addr->to_string()),
                              nullptr);
    }
    if (rsp->get_status() != HTTPStatus::SWITCHING_PROTOCOLS)
    {
        return std::make_pair(std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::INVALID_PROTOCOLS,
                                                           "not websocket server addr:" + addr->to_string()),
                              nullptr);
    }
    return std::make_pair(std::make_shared<HTTPResult>(rsp, HTTPResult::ErrorCode::OK, "ok"), conn);
}

std::pair<HTTPResult::ptr, WSConnection::ptr> WSConnection::Create(const std::string &url, uint64_t timeout_ms,
                                                                   const std::map<std::string, std::string> &headers)
{
    auto uri = URI::Create(url);
    if (!uri)
    {
        TIGER_LOG_E(SYSTEM_LOG) << "[Create fail"
                                << " url:" << url << "]";
        return std::make_pair(
            std::make_shared<HTTPResult>(nullptr, HTTPResult::ErrorCode::INVALID_URL, "invalid url:" + url), nullptr);
    }
    return Create(uri, timeout_ms, headers);
}

WSFrameMessage::ptr WSConnection::recv_message()
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
                TIGER_LOG_I(SYSTEM_LOG) << "[websocket handshake finished"
                                        << " data:" << data << "]";
                return std::make_shared<WSFrameMessage>(opcode, std::move(data));
            }
        }
        else if (ws_head.opcode == WSFrameHead::CLOSE)
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket closed remoteAddr:" << get_remote_address() << "]";
            break;
        }
        else
        {
            TIGER_LOG_I(SYSTEM_LOG) << "[websocket invalid opcode"
                                    << " opcode:" << opcode << "]";
        }
    } while (true);
    close();
    return nullptr;
}

int32_t WSConnection::send_message(WSFrameMessage::ptr msg, bool fin)
{
    do
    {
        WSFrameHead ws_head;
        memset(&ws_head, 0, sizeof(ws_head));
        ws_head.fin = fin;
        ws_head.opcode = msg->get_opcode();
        ws_head.mask = true;
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
        char mask[4];
        uint32_t rand_value = rand();
        memcpy(mask, &rand_value, sizeof(mask));
        std::string &data = msg->get_data();
        for (size_t i = 0; i < data.size(); ++i)
        {
            data[i] ^= mask[i % 4];
        }
        if (write_fixed_size(mask, sizeof(mask)) <= 0)
            break;
        if (write_fixed_size(msg->get_data().c_str(), size) <= 0)
            break;
        std::cout << "size === " << size << std::endl;
        return size + sizeof(ws_head);
    } while (false);
    close();
    return -1;
}

int32_t WSConnection::send_message(const std::string &msg, int32_t opcode, bool fin)
{
    return send_message(std::make_shared<WSFrameMessage>(opcode, msg), fin);
}

int32_t WSConnection::ping()
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

int32_t WSConnection::pong()
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