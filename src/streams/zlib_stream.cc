/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "zlib_stream.h"

#include <stdlib.h>

#include <stdexcept>

#include "../../src/macro.h"

namespace tiger
{

int ZlibStream::init(Type type, CompressLevel level, int window_bits, int mem_level, Strategy strategy)
{
    TIGER_ASSERT_WITH_INFO((level >= 0 && level <= 9) || level == CompressLevel::DEFAULT_COMPRESSION,
                           "[ZlibStream init level error]");
    TIGER_ASSERT_WITH_INFO(window_bits >= 8 && window_bits <= 15, "[ZlibStream init window bits error]");
    TIGER_ASSERT_WITH_INFO(mem_level >= 1 && mem_level <= 9, "[ZlibStream init mem level error]");
    memset(&m_zstream, 0, sizeof(m_zstream));
    m_zstream.zalloc = Z_NULL;
    m_zstream.zfree = Z_NULL;
    m_zstream.opaque = Z_NULL;
    switch (type)
    {
    case Type::DEFLATE:
        window_bits = -window_bits;
        break;
    case Type::GZIP:
        window_bits += 16;
        break;
    case Type::ZLIB:
        break;
    default:
        break;
    }
    if (m_encode)
    {
        int rt = deflateInit2(&m_zstream, level, Z_DEFLATED, window_bits, mem_level, (int)strategy);
        return rt;
    }
    else
    {
        int rt = inflateInit2(&m_zstream, window_bits);
        return rt;
    }
}

int ZlibStream::encode(const iovec *v, const uint32_t &len, bool finish)
{
    int ret = 0;
    int flush = 0;
    for (size_t i = 0; i < len; ++i)
    {
        m_zstream.avail_in = v[i].iov_len;
        m_zstream.next_in = (Bytef *)v[i].iov_base;
        flush = finish ? (i == len - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
        iovec *ivc = nullptr;
        do
        {
            if (!m_buffers.empty() && m_buffers.back().iov_len != m_buff_size)
            {
                ivc = &m_buffers.back();
            }
            else
            {
                iovec vc;
                vc.iov_base = malloc(m_buff_size);
                vc.iov_len = 0;
                m_buffers.push_back(vc);
                ivc = &m_buffers.back();
            }
            m_zstream.avail_out = m_buff_size - ivc->iov_len;
            m_zstream.next_out = (Bytef *)ivc->iov_base + ivc->iov_len;
            ret = deflate(&m_zstream, flush);
            if (ret == Z_STREAM_ERROR)
            {
                return ret;
            }
            ivc->iov_len = m_buff_size - m_zstream.avail_out;
        } while (m_zstream.avail_out == 0);
    }
    if (flush == Z_FINISH)
    {
        deflateEnd(&m_zstream);
    }
    return Z_OK;
}

int ZlibStream::decode(const iovec *v, const uint32_t &len, bool finish)
{
    int ret = 0;
    int flush = 0;
    for (size_t i = 0; i < len; ++i)
    {
        m_zstream.avail_in = v[i].iov_len;
        m_zstream.next_in = (Bytef *)v[i].iov_base;
        flush = finish ? (i == len - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
        iovec *ivc = nullptr;
        do
        {
            if (!m_buffers.empty() && m_buffers.back().iov_len != m_buff_size)
            {
                ivc = &m_buffers.back();
            }
            else
            {
                iovec vc;
                vc.iov_base = malloc(m_buff_size);
                vc.iov_len = 0;
                m_buffers.push_back(vc);
                ivc = &m_buffers.back();
            }
            m_zstream.avail_out = m_buff_size - ivc->iov_len;
            m_zstream.next_out = (Bytef *)ivc->iov_base + ivc->iov_len;
            ret = inflate(&m_zstream, flush);
            if (ret == Z_STREAM_ERROR)
            {
                return ret;
            }
            ivc->iov_len = m_buff_size - m_zstream.avail_out;
        } while (m_zstream.avail_out == 0);
    }
    if (flush == Z_FINISH)
    {
        inflateEnd(&m_zstream);
    }
    return Z_OK;
}

ZlibStream::ZlibStream(bool encode, uint32_t buff_size) : m_encode(encode), m_free(true), m_buff_size(buff_size)
{
}

ZlibStream::~ZlibStream()
{
    if (m_free)
    {
        for (auto &i : m_buffers)
        {
            free(i.iov_base);
        }
    }
    if (m_encode)
    {
        deflateEnd(&m_zstream);
    }
    else
    {
        inflateEnd(&m_zstream);
    }
}

ZlibStream::ptr ZlibStream::Create(bool encode, uint32_t buff_szie, Type type, CompressLevel level, int window_bits,
                                   int mem_level, Strategy strategy)
{
    ZlibStream::ptr rt = std::make_shared<ZlibStream>(encode, buff_szie);
    if (rt->init(type, level, window_bits, mem_level, strategy) == Z_OK)
    {
        return rt;
    }
    return nullptr;
}

ZlibStream::ptr ZlibStream::CreateGzip(bool encode, uint32_t buff_size)
{
    return Create(encode, buff_size, Type::GZIP);
}

ZlibStream::ptr ZlibStream::CreateZlib(bool encode, uint32_t buff_size)
{
    return Create(encode, buff_size, Type::ZLIB);
}

ZlibStream::ptr ZlibStream::CreateDeflate(bool encode, uint32_t buff_size)
{
    return Create(encode, buff_size, Type::DEFLATE);
}

int ZlibStream::read(void *buffer, size_t len)
{
    throw std::logic_error("[ZlibStream::read is invalid]");
}

int ZlibStream::read(ByteArray::ptr ba, size_t len)
{
    throw std::logic_error("[ZlibStream::read is invalid]");
}

int ZlibStream::write(const void *buffer, size_t len)
{
    iovec ivc;
    ivc.iov_base = (void *)buffer;
    ivc.iov_len = len;
    if (m_encode)
    {
        return encode(&ivc, 1, false);
    }
    else
    {
        return decode(&ivc, 1, false);
    }
}

int ZlibStream::write(ByteArray::ptr ba, size_t len)
{
    std::vector<iovec> buffers;
    ba->get_enable_read_buffers(buffers, len);
    if (m_encode)
    {
        return encode(&buffers[0], buffers.size(), false);
    }
    else
    {
        return decode(&buffers[0], buffers.size(), false);
    }
}

void ZlibStream::close()
{
    flush();
}

std::string ZlibStream::get_result() const
{
    std::string rt;
    for (auto &i : m_buffers)
    {
        rt.append((const char *)i.iov_base, i.iov_len);
    }
    return rt;
}

ByteArray::ptr ZlibStream::get_byte_array() const
{
    ByteArray::ptr ba = std::make_shared<ByteArray>();
    for (auto &i : m_buffers)
    {
        ba->write(i.iov_base, i.iov_len);
    }
    ba->set_position(0);
    return ba;
}

int ZlibStream::flush()
{
    iovec ivc;
    ivc.iov_base = nullptr;
    ivc.iov_len = 0;
    if (m_encode)
    {
        return encode(&ivc, 1, true);
    }
    else
    {
        return decode(&ivc, 1, true);
    }
}

} // namespace tiger