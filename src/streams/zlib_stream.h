/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/11/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGER_STREAM_ZLIB_STREAM_H__
#define __TIGER_STREAM_ZLIB_STREAM_H__

#include <zlib.h>

#include "stream.h"

namespace tiger {

class ZlibStream : public Stream {
   public:
    enum Type {
        ZLIB,
        DEFLATE,
        GZIP
    };

    enum Strategy {
        DEFAULT = Z_DEFAULT_STRATEGY,
        FILTERED = Z_FILTERED,
        HUFFMAN = Z_HUFFMAN_ONLY,
        FIXED = Z_FIXED,
        RLE = Z_RLE
    };

    enum CompressLevel {
        NO_COMPRESSION = Z_NO_COMPRESSION,
        BEST_SPEED = Z_BEST_SPEED,
        BEST_COMPRESSION = Z_BEST_COMPRESSION,
        DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
    };

   private:
    bool m_encode;
    bool m_free;
    uint32_t m_buff_size;
    z_stream m_zstream;
    std::vector<iovec> m_buffers;

   private:
    int init(Type type = Type::DEFLATE,
             CompressLevel level = CompressLevel::DEFAULT_COMPRESSION,
             int window_bits = 15,
             int mem_level = 8,
             Strategy strategy = Strategy::DEFAULT);
    int encode(const iovec *v, const uint32_t &len, bool finish);
    int decode(const iovec *v, const uint32_t &len, bool finish);

   public:
    typedef std::shared_ptr<ZlibStream> ptr;

    ZlibStream(bool encode, uint32_t buff_size = 4096);
    ~ZlibStream();

    static ZlibStream::ptr Create(bool encode,
                                  uint32_t buff_szie = 4096,
                                  Type type = Type::DEFLATE,
                                  CompressLevel level = CompressLevel::DEFAULT_COMPRESSION,
                                  int window_bits = 15,
                                  int mem_level = 8,
                                  Strategy strategy = Strategy::DEFAULT);
    static ZlibStream::ptr CreateGzip(bool encode, uint32_t buff_size = 4096);
    static ZlibStream::ptr CreateZlib(bool encode, uint32_t buff_size = 4096);
    static ZlibStream::ptr CreateDeflate(bool encode, uint32_t buff_size = 4096);

   public:
    bool is_free() const { return m_free; }
    void set_free(bool v) { m_free = v; }
    bool is_encode() const { return m_encode; }
    void set_encode(bool v) { m_encode = v; }
    std::vector<iovec> &get_buffers() { return m_buffers; }

    std::string get_result() const;
    ByteArray::ptr get_byte_array() const;

   public:
    int flush();

   public:
    virtual int read(void *buffer, size_t len) override;
    virtual int read(ByteArray::ptr ba, size_t len) override;
    virtual int write(const void *buffer, size_t len) override;
    virtual int write(ByteArray::ptr ba, size_t len) override;

    virtual void close() override;
};

}  // namespace tiger

#endif
