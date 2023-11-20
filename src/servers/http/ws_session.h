/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/02
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_HTTP_WS_SESSION_H__
#define __TIGER_HTTP_WS_SESSION_H__

#include <stdint.h>

#include "../../config.h"
#include "http_session.h"

namespace tiger
{

namespace http
{

// https://datatracker.ietf.org/doc/html/rfc6455#section-5.2
#pragma pack(1)
struct WSFrameHead
{
    enum
    {
        CONTINUE = 0,
        TEXT_FRAME = 1,
        BIN_FRAME = 2,
        CLOSE = 8,
        PING = 9,
        PONG = 0xA
    };
    uint32_t opcode : 4;
    bool rsv3 : 1;
    bool rsv2 : 1;
    bool rsv1 : 1;
    bool fin : 1;
    uint32_t payload : 7;
    bool mask : 1;

    std::string to_string() const;
};
#pragma pack()

class WSFrameMessage
{
  private:
    int m_opcode;
    std::string m_data;

  public:
    typedef std::shared_ptr<WSFrameMessage> ptr;

    WSFrameMessage(int opcode = 0, const std::string &data = "");
    ~WSFrameMessage(){};

  public:
    int get_opcode() const
    {
        return m_opcode;
    }
    void set_opcode(int opcode)
    {
        m_opcode = opcode;
    }

    const std::string &get_data() const
    {
        return m_data;
    }
    std::string &get_data()
    {
        return m_data;
    }
    void set_data(const std::string &data)
    {
        m_data = data;
    }
};

class WSSession : public HTTPSession
{
  private:
    bool handle_server_shake();
    bool handle_client_shake();

  public:
    typedef std::shared_ptr<WSSession> ptr;

    WSSession(Socket::ptr socket, bool owner = true);
    ~WSSession() = default;

    HTTPRequest::ptr handle_shake();
    WSFrameMessage::ptr recv_message();
    int32_t send_message(WSFrameMessage::ptr msg, bool fin = true);
    int32_t send_message(const std::string &msg, int32_t opcode = WSFrameHead::TEXT_FRAME, bool fin = true);
    int32_t ping();
    int32_t pong();
};

} // namespace http

} // namespace tiger

#endif